#pragma once

#include "block.h"

namespace Blocks {

class Button : public Interactive {
public:
	struct Properties {
		Drawable* default_v = nullptr; // Must not be null, all others can be.
		Drawable* hover_v = nullptr;
		Drawable* click_v = nullptr;
		Drawable* disabled_v = nullptr;
	};

	// For buttons with only one visual:
	explicit Button(Drawable& vis) : Interactive(vis.width(), vis.height()) {
		props_.default_v = &vis;
	}
	
	explicit Button(Properties props) : Interactive(props.default_v->width(), props.default_v->height()), props_(props) {}

	void event(BlockEvent e) override;
	void action(BlockAction a) override; // Actions: Enable, Disable, etc.
	void draw() override;

protected:
	Properties props_;

	static const uint8_t StateDefault = 0;
	static const uint8_t StateHover = 1;
	static const uint8_t StateClick = 2;
	static const uint8_t StateDisabled = 3;

	uint8_t state_ = StateDefault;

	bool hovered_ = false;
	bool pressed_down_ = false; // So drag out + up or drag in + up don't generate presses.
};

} // namespace Blocks
