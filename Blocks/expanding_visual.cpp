#include "expanding_visual.h"

namespace Blocks {

void ExpandingVisual::draw() {
	bool drawn = false;
	if (!props_.background_color.is_transparent()) {
		render.DrawRect(width_, height_, props_.background_color);
		drawn = true;
	}
	if (props_.sprite != nullptr) {
		if (props_.sprite_tile_mode) {
			render.DrawSpriteTiled(*(props_.sprite), width_, height_);
		} else {
			render.DrawSpriteStretch(*(props_.sprite), width_, height_);
		}
	} else if (props_.ondraw_func != nullptr) {
		props_.ondraw_func(width_, height_);
	} else if (!drawn) {
		log::Error("ExpandingVisual", "Attempted to draw a null expanding visual");
	}
}

} // namespace Blocks
