#pragma once

#include "block.h"

namespace Blocks {

class ExpandingVisual : public ExpandingDrawable {
public:
	struct Properties {
		// SET ONLY ONE OF THESE: + All NOT Owned:
		void (*ondraw_func)(int, int) = nullptr; // Passes the current width, height needed to draw
		Sprite* sprite = nullptr;

		// This can also be set for a background color drawn first. (Useful for rectangles, etc. as well)
		Color background_color;

		bool sprite_tile_mode = true; // Otherwise, it will stretch the sprite to the available size
	};

	ExpandingVisual(const Properties& props, const ExpandingProperties& ex_props) : ExpandingDrawable(ex_props), props_(props) {}

	void draw() override; // Coordinates to draw

	void setBackgroundColor(const Color& bg_color) { props_.background_color = bg_color; }

protected:
	Properties props_;

	// TODO: Transform for stretching/scaling/etc.
};

} // namespace Blocks
