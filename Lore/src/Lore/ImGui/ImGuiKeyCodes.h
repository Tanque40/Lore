#pragma once

#include "imgui.h"

#include "Lore/KeyCodes.h"

namespace Lore {
	// Función para mapear códigos de teclas GLFW a ImGuiKey
	static ImGuiKey KeyToImGuiKey(int keycode) {
		switch (keycode) {
		case LR_KEY_TAB: return ImGuiKey_Tab;
		case LR_KEY_LEFT: return ImGuiKey_LeftArrow;
		case LR_KEY_RIGHT: return ImGuiKey_RightArrow;
		case LR_KEY_UP: return ImGuiKey_UpArrow;
		case LR_KEY_DOWN: return ImGuiKey_DownArrow;
		case LR_KEY_PAGE_UP: return ImGuiKey_PageUp;
		case LR_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
		case LR_KEY_HOME: return ImGuiKey_Home;
		case LR_KEY_END: return ImGuiKey_End;
		case LR_KEY_INSERT: return ImGuiKey_Insert;
		case LR_KEY_DELETE: return ImGuiKey_Delete;
		case LR_KEY_BACKSPACE: return ImGuiKey_Backspace;
		case LR_KEY_SPACE: return ImGuiKey_Space;
		case LR_KEY_ENTER: return ImGuiKey_Enter;
		case LR_KEY_ESCAPE: return ImGuiKey_Escape;
		case LR_KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
		case LR_KEY_COMMA: return ImGuiKey_Comma;
		case LR_KEY_MINUS: return ImGuiKey_Minus;
		case LR_KEY_PERIOD: return ImGuiKey_Period;
		case LR_KEY_SLASH: return ImGuiKey_Slash;
		case LR_KEY_SEMICOLON: return ImGuiKey_Semicolon;
		case LR_KEY_EQUAL: return ImGuiKey_Equal;
		case LR_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
		case LR_KEY_BACKSLASH: return ImGuiKey_Backslash;
		case LR_KEY_WORLD_1: return ImGuiKey_Oem102;
		case LR_KEY_WORLD_2: return ImGuiKey_Oem102;
		case LR_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
		case LR_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
		case LR_KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
		case LR_KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
		case LR_KEY_NUM_LOCK: return ImGuiKey_NumLock;
		case LR_KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
		case LR_KEY_PAUSE: return ImGuiKey_Pause;
		case LR_KEY_KP_0: return ImGuiKey_Keypad0;
		case LR_KEY_KP_1: return ImGuiKey_Keypad1;
		case LR_KEY_KP_2: return ImGuiKey_Keypad2;
		case LR_KEY_KP_3: return ImGuiKey_Keypad3;
		case LR_KEY_KP_4: return ImGuiKey_Keypad4;
		case LR_KEY_KP_5: return ImGuiKey_Keypad5;
		case LR_KEY_KP_6: return ImGuiKey_Keypad6;
		case LR_KEY_KP_7: return ImGuiKey_Keypad7;
		case LR_KEY_KP_8: return ImGuiKey_Keypad8;
		case LR_KEY_KP_9: return ImGuiKey_Keypad9;
		case LR_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
		case LR_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
		case LR_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
		case LR_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
		case LR_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
		case LR_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
		case LR_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
		case LR_KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
		case LR_KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
		case LR_KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
		case LR_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
		case LR_KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
		case LR_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
		case LR_KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
		case LR_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
		case LR_KEY_MENU: return ImGuiKey_Menu;
		case LR_KEY_0: return ImGuiKey_0;
		case LR_KEY_1: return ImGuiKey_1;
		case LR_KEY_2: return ImGuiKey_2;
		case LR_KEY_3: return ImGuiKey_3;
		case LR_KEY_4: return ImGuiKey_4;
		case LR_KEY_5: return ImGuiKey_5;
		case LR_KEY_6: return ImGuiKey_6;
		case LR_KEY_7: return ImGuiKey_7;
		case LR_KEY_8: return ImGuiKey_8;
		case LR_KEY_9: return ImGuiKey_9;
		case LR_KEY_A: return ImGuiKey_A;
		case LR_KEY_B: return ImGuiKey_B;
		case LR_KEY_C: return ImGuiKey_C;
		case LR_KEY_D: return ImGuiKey_D;
		case LR_KEY_E: return ImGuiKey_E;
		case LR_KEY_F: return ImGuiKey_F;
		case LR_KEY_G: return ImGuiKey_G;
		case LR_KEY_H: return ImGuiKey_H;
		case LR_KEY_I: return ImGuiKey_I;
		case LR_KEY_J: return ImGuiKey_J;
		case LR_KEY_K: return ImGuiKey_K;
		case LR_KEY_L: return ImGuiKey_L;
		case LR_KEY_M: return ImGuiKey_M;
		case LR_KEY_N: return ImGuiKey_N;
		case LR_KEY_O: return ImGuiKey_O;
		case LR_KEY_P: return ImGuiKey_P;
		case LR_KEY_Q: return ImGuiKey_Q;
		case LR_KEY_R: return ImGuiKey_R;
		case LR_KEY_S: return ImGuiKey_S;
		case LR_KEY_T: return ImGuiKey_T;
		case LR_KEY_U: return ImGuiKey_U;
		case LR_KEY_V: return ImGuiKey_V;
		case LR_KEY_W: return ImGuiKey_W;
		case LR_KEY_X: return ImGuiKey_X;
		case LR_KEY_Y: return ImGuiKey_Y;
		case LR_KEY_Z: return ImGuiKey_Z;
		case LR_KEY_F1: return ImGuiKey_F1;
		case LR_KEY_F2: return ImGuiKey_F2;
		case LR_KEY_F3: return ImGuiKey_F3;
		case LR_KEY_F4: return ImGuiKey_F4;
		case LR_KEY_F5: return ImGuiKey_F5;
		case LR_KEY_F6: return ImGuiKey_F6;
		case LR_KEY_F7: return ImGuiKey_F7;
		case LR_KEY_F8: return ImGuiKey_F8;
		case LR_KEY_F9: return ImGuiKey_F9;
		case LR_KEY_F10: return ImGuiKey_F10;
		case LR_KEY_F11: return ImGuiKey_F11;
		case LR_KEY_F12: return ImGuiKey_F12;
		case LR_KEY_F13: return ImGuiKey_F13;
		case LR_KEY_F14: return ImGuiKey_F14;
		case LR_KEY_F15: return ImGuiKey_F15;
		case LR_KEY_F16: return ImGuiKey_F16;
		case LR_KEY_F17: return ImGuiKey_F17;
		case LR_KEY_F18: return ImGuiKey_F18;
		case LR_KEY_F19: return ImGuiKey_F19;
		case LR_KEY_F20: return ImGuiKey_F20;
		case LR_KEY_F21: return ImGuiKey_F21;
		case LR_KEY_F22: return ImGuiKey_F22;
		case LR_KEY_F23: return ImGuiKey_F23;
		case LR_KEY_F24: return ImGuiKey_F24;
		default: return ImGuiKey_None;
		}
	}
}
