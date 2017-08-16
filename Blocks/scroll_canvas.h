#pragma once

#include "canvas.h"

namespace Blocks {

class ScrollCanvas : public Canvas, public FrameProcessor {
public:
	struct ScrollProperties {
		typedef uint8_t Type;

		// Used for auto-scrolling, in px per sec
		float speed_x = 0.0f;
		float speed_y = 0.0f;

		int start_x = 0;
		int start_y = 0;

		// Scroll limits:
		int min_x = 0; // Can be negative
		int min_y = 0;
		int max_x = 0; // 0 means no limit for max, cannot be negative
		int max_y = 0;

		static const Type NO_SCROLL = 0;
		static const Type AUTO = 1;
		static const Type CENTER = 2; // TODO //

		static const Type WRAP_NONE = 0;
		static const Type WRAP_TOROIDAL = 1;

		Type type_x = NO_SCROLL;
		Type type_y = NO_SCROLL;
		Type wrap_x = WRAP_NONE;
		Type wrap_y = WRAP_NONE;
	};

	ScrollCanvas(ScrollProperties sc_props, Properties props, ExpandingProperties ex_props)
		: Canvas(props, ex_props), sc_props_(sc_props) {
		resetScrollToStart();
	}

	BlockElement* addScrollDrawableAt(Drawable& block, const int x, const int y);
	BlockElement* addScrollInteractiveAt(Interactive& block, const int x, const int y);

	void removeScrollBlock(Drawable& block);
	void removeScrollBlock(BlockElement* block_element);

	void setScrollPos(const int x, const int y);
	void moveScrollBy(const int dx, const int dy);
	void setScrollSpeed(const float speed_x, const float speed_y); // In px per sec.
	void setScrollStart(const int x, const int y);

	void resetScrollToStart();

	void pauseScroll() { scrolling_ = false; }
	void resumeScroll() { scrolling_ = true; }

	void event(BlockEvent e) override;
	void action(BlockAction a) override;
	
	void draw() override;

	void frame(double frame_delta_time_msec) override;

protected:
	void CalcWrapValues();
	void GetScrollCoords(BlockElement* be, const Drawable* d, int& x, int& y, int& xw, int& yh);

	ScrollProperties sc_props_;

	// All scrolling drawables and interactives (are drawn here, others drawn by the Canvas.)
	// Owned by this vector:
	std::vector<BlockElement*> sc_drawables_;
	// Subset of the above, for fast screen event processing: (NOT Owned)
	std::vector<BlockElement*> sc_interactives_;

	float rem_x_ = 0.0f;
	float rem_y_ = 0.0f;

	// Current scroll values:
	int sc_x_ = 0;
	int sc_y_ = 0;

	int wrap_total_x_ = 0;
	int wrap_total_y_ = 0;

	bool wrap_x_max_ = false;
	bool wrap_y_max_ = false;

	bool scrolling_ = true;
};

} // namespace Blocks
