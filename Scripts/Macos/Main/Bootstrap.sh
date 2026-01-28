# Setup premake

cd vendor/premake
make -f Bootstrap.mak macosx-clean
make -f Bootstrap.mak macosx PLATFORM=ARM64
cd ../../

# Setup ronin project with premake

./vendor/premake/bin/release/premake5 gmake
