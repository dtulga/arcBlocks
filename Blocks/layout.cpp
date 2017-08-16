#include "layout.h"

#include <algorithm>

namespace Blocks {

// Helper class to lay out blocks effectively, but does not actually handle drawing or events.

// Always adds x and y diff, no rows/columns
void Layout::setLayoutLine(const int start_x, const int start_y, const int x_diff, const int y_diff, const uint8_t grid_mode) {
	start_x_ = start_x;
	start_y_ = start_y;
	next_x_ = start_x;
	next_y_ = start_y;
	x_diff_ = x_diff;
	y_diff_ = y_diff;
	grid_mode_ = grid_mode;
	is_line_ = true;
}

// Adds x_diff until nextRow() is called, then resets x and adds y diff
void Layout::setLayoutGrid(const int start_x, const int start_y, const int x_diff, const int y_diff, const uint8_t grid_mode) {
	start_x_ = start_x;
	start_y_ = start_y;
	next_x_ = start_x;
	next_y_ = start_y;
	x_diff_ = x_diff;
	y_diff_ = y_diff;
	grid_mode_ = grid_mode;
	is_line_ = false;
}

// Must set a layout to use these
void Layout::addDrawable(Drawable& block) {
	int x; int y;

	getCurCoords(block, x, y);	

	canvas_.addDrawableAt(block, x, y);

	next(block);
}

void Layout::addInteractive(Interactive& block) {
	int x; int y;

	getCurCoords(block, x, y);

	canvas_.addInteractiveAt(block, x, y);

	next(block);
}

void Layout::getCurCoords(Drawable& block, int& x, int& y) {
	x = next_x_;
	y = next_y_;

	if (grid_mode_ == GRID_MODE_EXACT_TOP_RIGHT || grid_mode_ == GRID_MODE_EXACT_BOTTOM_RIGHT) {
		x -= block.width();
	}
	if (grid_mode_ == GRID_MODE_EXACT_BOTTOM_LEFT || grid_mode_ == GRID_MODE_EXACT_BOTTOM_RIGHT) {
		y -= block.height();
	}
	if (grid_mode_ == GRID_MODE_EXACT_CENTER) {
		x -= block.width() / 2;
		y -= block.height() / 2;
	}

	setPrevCoords(block, x, y);
}

void Layout::setPrevCoords(Drawable& block, const int x, const int y) {
	prev_x_ = x;
	prev_y_ = y;
	prev_width_ = block.width();
	prev_height_ = block.height();
}

void Layout::next(Drawable& block) {
	if (grid_mode_ == GRID_MODE_SPACING) {
		next_x_ += x_diff_ + block.width();
		if (is_line_) {
			next_y_ += y_diff_ + block.height();
		} else {
			if (spacing_min_) {
				row_spacing_ = (row_spacing_ == 0) ? block.height() : std::min(row_spacing_, block.height());
			} else {
				row_spacing_ = std::max(row_spacing_, block.height());
			}
		}
	} else { // All exacts
		next_x_ += x_diff_;
		if (is_line_) {
			next_y_ += y_diff_;
		}
	}
}

// Only used for grid layout
void Layout::nextRow() {
	if (grid_mode_ == GRID_MODE_SPACING) {
		next_y_ += y_diff_ + row_spacing_;
	} else {
		next_y_ += y_diff_;
	}
	next_x_ = start_x_;
}

// Can be used without a layout set
void Layout::addDrawableAt(Drawable& block, const int x, const int y) {
	setPrevCoords(block, x, y);
	canvas_.addDrawableAt(block, x, y);
}

void Layout::addInteractiveAt(Interactive& block, const int x, const int y) {
	setPrevCoords(block, x, y);
	canvas_.addInteractiveAt(block, x, y);
}

void Layout::getSpacedFromPrevCoords(Drawable& block, int& x, int& y) {
	if (x > 0) {
		x += prev_width_;
	} else {
		x -= block.width();
	}
	if (y > 0) {
		y += prev_height_;
	} else {
		y -= block.height();
	}

	x += prev_x_;
	y += prev_y_;

	setPrevCoords(block, x, y);
}

// Automatically does: +x from right edge, -x from left edge, +y from bottom edge, -y from top edge
void Layout::addDrawableSpacedFromPrev(Drawable& block, int x, int y) {
	getSpacedFromPrevCoords(block, x, y);
	canvas_.addDrawableAt(block, x, y);
}

// Automatically does: +x from right edge, -x from left edge, +y from bottom edge, -y from top edge
void Layout::addInteractiveSpacedFromPrev(Interactive& block, int x, int y) {
	getSpacedFromPrevCoords(block, x, y);
	canvas_.addInteractiveAt(block, x, y);
}

} // namespace Blocks
