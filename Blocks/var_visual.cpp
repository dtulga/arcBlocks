#include "var_visual.h"

namespace Blocks {

void VarVisual::draw() {
	bool drawn = false;
	if (!props_.background_color.is_transparent()) {
		render.DrawRect(width_, height_, props_.background_color);
		drawn = true;
	}

	if (props_.int_var != nullptr) {
		const string& str = props_.int_var->toString();

		if (str.len() < props_.pad_to_length) {
			// This should work with no allocations/deallocations.
			buffer_.assign(props_.pad_with_char, props_.pad_to_length - str.len());

			buffer_.append(str);

			render.DrawPixelText(buffer_, props_.pixel_font_id);
		} else {
			render.DrawPixelText(str, props_.pixel_font_id);
		}
	} else if (!drawn) {
		log::Error("VarVisual", "Attempted to draw a null VarVisual");
	}
}

} // namespace Blocks
