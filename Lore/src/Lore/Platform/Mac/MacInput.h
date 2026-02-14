#pragma once

#ifdef LORE_PLATFORM_MAC

#include "Lore/Input.h"

namespace Lore {
	class MacInput : public Input {
	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;

		virtual bool IsMouseButtonPressedImpl(int buttoncode) override;
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}

#endif
