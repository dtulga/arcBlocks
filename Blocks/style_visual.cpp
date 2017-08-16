#include "style_visual.h"

namespace Blocks {

void StyleVisual::draw() {
	int off_x = props_.margin_left;
	int off_y = props_.margin_top;
	int w = width_ - props_.margin_right - off_x;
	int h = height_ - props_.margin_bottom - off_y;

	int i = props_.border_thickness;

	while (i > 0) {
		if (!props_.border_color.is_transparent()) {
			render.DrawRectBorder(off_x, off_y, w, h, props_.border_color);
		}
		off_x++;
		off_y++;
		w -= 2;
		h -= 2;
	}

	if (!props_.background_color.is_transparent()) {
		render.DrawRect(off_x, off_y, w, h);
	}
	
	if (visual_ != nullptr) {
		render.AddDrawOffset(off_x + props_.padding_left, off_y + props_.padding_top);

		visual_->draw();
	}
}

} // namespace Blocks