#pragma once

#ifdef LORE_PLATFORM_WINDOWS

#include "Lore/Input.h"

namespace Lore {
	class WindowsInput : public Input {
	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;

		virtual bool IsMouseButtonPressedImpl(int buttoncode) override;
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}

#endif
