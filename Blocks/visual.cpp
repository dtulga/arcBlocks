#include "visual.h"

namespace Blocks {

// TODO: Error checking if ondraw is already set? //
void Visual::setSprite(Sprite& s) {
	props_.sprite = &s;
	width_ = s.width();
	height_ = s.height();
}

void Visual::draw() {
	bool drawn = false;
	if (!props_.background_color.is_transparent()) {
		render.DrawRect(width_, height_, props_.background_color);
		drawn = true;
	}
	if (props_.sprite != nullptr) {
		render.DrawSprite(*props_.sprite);
	} else if (props_.ondraw_func != nullptr) {
		props_.ondraw_func();
	} else if (!props_.pixel_text.empty()) {
		render.DrawPixelText(props_.pixel_text, props_.pixel_font_id);
	} else if (!drawn) {
		log::Error("Visual", "Attempted to draw a null visual");
	}
}

} // namespace Blocks
