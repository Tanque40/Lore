#!/usr/bin/env python3
"""Compila los shaders de una sola fuente (GLSL 460) para todos los backends.

Por cada Sandbox/src/Shaders/src/<Nombre>.<etapa> (comp, vert, frag) genera en
Sandbox/src/Shaders/bin/:

  <Nombre>.<etapa>.gl.spv   SPIR-V para OpenGL (glslangValidator -G). Windows lo carga con
                            GL_ARB_gl_spirv.
  <Nombre>.<etapa>.gl.glsl  GLSL regenerado por spirv-cross a partir de ese SPIR-V. Sólo es
                            el respaldo si un driver no soporta GL_ARB_gl_spirv.
  <Nombre>.<etapa>.metal    MSL para Metal (glslangValidator -V + spirv-cross --msl). Los
                            índices [[buffer(n)]] / [[texture(n)]] son los bindings del GLSL.
  <Nombre>.<etapa>.hash     Hash de la fuente y los headers con que se generó. Sólo se
                            recompila si cambia (las fechas de archivo no cuentan).

Los archivos generados se versionan en git para que el proyecto compile aunque no tengas
las herramientas instaladas. Si editas un shader necesitas:

  macOS:   brew install glslang spirv-cross
  Windows: Vulkan SDK (trae glslangValidator y spirv-cross; define VULKAN_SDK)
  Linux:   apt install glslang-tools spirv-cross

Uso: python3 Scripts/CompileShaders.py [--force] [--check]
  --force  recompila todo aunque nada haya cambiado
  --check  no escribe nada; sale con 1 si algún shader no compila (para CI)

premake lo llama antes de compilar el Sandbox. Si faltan las herramientas y los archivos
generados ya existen, avisa y sigue (no rompe el build); si faltan los generados, falla.
"""

import argparse
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
SRC_DIR = os.path.join(ROOT, "Sandbox", "src", "Shaders", "src")
OUT_DIR = os.path.join(ROOT, "Sandbox", "src", "Shaders", "bin")
INCLUDE_DIR = os.path.join(ROOT, "Lore", "src")
# Headers compartidos C++/GLSL: si cambian, hay que recompilar todo.
SHARED_HEADERS = [os.path.join(INCLUDE_DIR, "Lore", "World", "VoxelShaderTypes.h")]

STAGES = {"comp": "comp", "vert": "vert", "frag": "frag"}
# Nombre de la función de entrada en Metal por etapa (MetalComputeShader busca compute_main).
MSL_ENTRY = {"comp": "compute_main", "vert": "vertex_main", "frag": "fragment_main"}


def find_tool(name):
    path = shutil.which(name)
    if path:
        return path
    sdk = os.environ.get("VULKAN_SDK")
    if sdk:
        for sub in ("bin", "Bin"):
            for candidate in (name, name + ".exe"):
                full = os.path.join(sdk, sub, candidate)
                if os.path.isfile(full):
                    return full
    return None


def run(cmd):
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        sys.stderr.write(" ".join(cmd) + "\n" + result.stdout + result.stderr)
        return False
    return True


def outputs_for(source):
    base = os.path.join(OUT_DIR, os.path.basename(source))
    return [base + ".gl.spv", base + ".gl.glsl", base + ".metal"]


def stamp_path(source):
    return os.path.join(OUT_DIR, os.path.basename(source) + ".hash")


def input_hash(source):
    # Hash del contenido (fuente + headers compartidos), no de las fechas: git checkout
    # cambia las fechas y no queremos regenerar (con otra versión de spirv-cross) archivos
    # cuya fuente no cambió.
    digest = hashlib.sha256()
    for path in [source] + SHARED_HEADERS:
        if os.path.isfile(path):
            with open(path, "rb") as f:
                digest.update(f.read().replace(b"\r\n", b"\n"))
    return digest.hexdigest()


def is_stale(source, outputs):
    if not all(os.path.isfile(o) for o in outputs):
        return True
    try:
        with open(stamp_path(source)) as f:
            return f.read().strip() != input_hash(source)
    except OSError:
        return True


def compile_shader(source, stage, glslang, spirv_cross, out_paths):
    gl_spv, gl_glsl, metal = out_paths
    with tempfile.TemporaryDirectory() as tmp:
        vk_spv = os.path.join(tmp, "vk.spv")
        include = "-I" + INCLUDE_DIR
        ok = run([glslang, "-G", "-S", stage, include, "-o", gl_spv, source])
        ok = ok and run([glslang, "-V", "-S", stage, include, "-o", vk_spv, source])
        ok = ok and run([spirv_cross, gl_spv, "--version", "460", "--output", gl_glsl])
        ok = ok and run([spirv_cross, vk_spv, "--msl", "--msl-version", "20100",
                         "--msl-decoration-binding",
                         "--rename-entry-point", "main", MSL_ENTRY[stage], stage,
                         "--output", metal])
        return ok


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    sources = []
    for name in sorted(os.listdir(SRC_DIR)):
        ext = name.rsplit(".", 1)[-1]
        if ext in STAGES:
            sources.append((os.path.join(SRC_DIR, name), STAGES[ext]))

    glslang = find_tool("glslangValidator")
    spirv_cross = find_tool("spirv-cross")
    os.makedirs(OUT_DIR, exist_ok=True)

    failed = False
    for source, stage in sources:
        outputs = outputs_for(source)
        name = os.path.relpath(source, ROOT)

        if not args.check and not args.force and not is_stale(source, outputs):
            continue

        if not glslang or not spirv_cross:
            missing = [n for n, t in (("glslangValidator", glslang), ("spirv-cross", spirv_cross)) if not t]
            if all(os.path.isfile(o) for o in outputs) and not args.check:
                print("[shaders] AVISO: %s cambió pero falta %s; se usan los compilados que ya están en "
                      "Shaders/bin (ver Scripts/CompileShaders.py para instalarlos)" % (name, ", ".join(missing)))
                continue
            print("[shaders] ERROR: falta %s para compilar %s" % (", ".join(missing), name))
            failed = True
            continue

        if args.check:
            with tempfile.TemporaryDirectory() as tmp:
                tmp_outputs = [os.path.join(tmp, os.path.basename(o)) for o in outputs]
                ok = compile_shader(source, stage, glslang, spirv_cross, tmp_outputs)
        else:
            ok = compile_shader(source, stage, glslang, spirv_cross, outputs)
            if ok:
                with open(stamp_path(source), "w") as f:
                    f.write(input_hash(source) + "\n")

        print("[shaders] %s %s" % ("OK   " if ok else "ERROR", name))
        failed = failed or not ok

    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
