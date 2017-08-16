#include "compound_visual.h"

namespace Blocks {

void CompoundVisual::draw() {
	bool drawn = false;
	if (!props_.background_color.is_transparent()) {
		render.DrawRect(width_, height_, props_.background_color);
		drawn = true;
	}

	const size_t len = props_.visuals.size();
	if (len == 0 && !drawn) {
		log::Error("CompoundVisual", "Attempted to draw a null compound visual");
		return;
	}

	int off_x = 0;
	int off_y = 0;
	render.GetDrawOffset(off_x, off_y);

	for (size_t i = 0; i < len; i++) {
		BasicAlignment& a = props_.visuals[i];
		Drawable* d = a.d;
		const int x = a.x();
		const int y = a.y();

		int draw_x = off_x;
		int draw_y = off_y;
		
		if (x == AlignCentered) {
			draw_x += (width_ / 2) - (d->width() / 2);
		} else if (x < 0) {
			draw_x += width_ - x - d->width();
		} else {
			draw_x += x;
		}

		if (y == AlignCentered) {
			draw_y += (height_ / 2) - (d->height() / 2);
		} else if (y < 0) {
			draw_y += height_ - y - d->height();
		} else {
			draw_y += y;
		}

		render.SetDrawOffset(draw_x, draw_y);
		d->draw();
	}

	render.SetDrawOffset(off_x, off_y); // TODO: This may be unnecessary?
}

} // namespace Blocks
