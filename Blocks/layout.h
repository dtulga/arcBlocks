#pragma once

#include "block.h"
#include "canvas.h"

namespace Blocks {

// Helper class to lay out blocks effectively, but does not actually handle drawing or events.
class Layout {
public:
	explicit Layout(Canvas& canvas) : canvas_(canvas) {}

	void setCanvas(Canvas& canvas) { canvas_ = canvas; }

	// Always adds x and y diff, no rows/columns
	void setLayoutLine(const int start_x, const int start_y, const int x_diff, const int y_diff, const uint8_t grid_mode = GRID_MODE_SPACING);
	// Adds x_diff until nextRow() is called, then resets x and adds y diff
	void setLayoutGrid(const int start_x, const int start_y, const int x_diff, const int y_diff, const uint8_t grid_mode = GRID_MODE_SPACING);

	// Must set a layout to use these
	void addDrawable(Drawable& block);
	void addInteractive(Interactive& block);

	// Only used for grid layout
	void nextRow();

	void reset(); // Go back to the beginning, for example to use the same layout on multiple canvases/groups, etc.

	// Can be used without a layout set
	void addDrawableAt(Drawable& block, const int x, const int y);
	void addInteractiveAt(Interactive& block, const int x, const int y);

	// Automatically does:
	// +x from right edge, -x from left edge
	// +y from bottom edge, -y from top egde
	void addDrawableSpacedFromPrev(Drawable& block, int x, int y);
	void addInteractiveSpacedFromPrev(Interactive& block, int x, int y);

	// Adds x and y diff after the elements' sizes.
	static const uint8_t GRID_MODE_SPACING = 0; // Spacing between max element widths for rows.
	static const uint8_t GRID_MODE_SPACING_MIN = 1; // Spacing between min element widths for rows.
	// TODO: x_diff / y_diff negative working correctly with grid spacing! (Already works with exact and ...SpacedFromPrev) //

	// Adds x and y diff then aligns each element like this.
	static const uint8_t GRID_MODE_EXACT_CENTER = 10;
	static const uint8_t GRID_MODE_EXACT_TOP_LEFT = 11;
	static const uint8_t GRID_MODE_EXACT_TOP_RIGHT = 12;
	static const uint8_t GRID_MODE_EXACT_BOTTOM_LEFT = 13;
	static const uint8_t GRID_MODE_EXACT_BOTTOM_RIGHT = 14;

protected:
	void getCurCoords(Drawable& block, int& x, int& y); // Also auto-calls setPrevCoords.
	void getSpacedFromPrevCoords(Drawable& block, int& x, int& y); // Also auto-calls setPrevCoords.
	void setPrevCoords(Drawable& block, const int x, const int y);
	void next(Drawable& block);

	Canvas& canvas_;

	int start_x_ = 0;
	int start_y_ = 0;
	int next_x_ = 0;
	int next_y_ = 0;
	int x_diff_ = 0;
	int y_diff_ = 0;

	// These are always stored as top-left style coordinates.
	int prev_x_ = 0;
	int prev_y_ = 0;
	int prev_width_ = 0;
	int prev_height_ = 0;

	int row_spacing_ = 0;

	uint8_t grid_mode_ = 0;
	bool is_line_ = false;
	bool spacing_min_ = false;
};

} // namespace Blocks
