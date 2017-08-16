#pragma once

#include <vector>

#include "scroll_canvas.h"

namespace Blocks {

class ParallaxScrollGroup : public ExpandingInteractive {
public:
	explicit ParallaxScrollGroup(ScrollCanvas& first_canvas);

	void addCanvasWithFactor(ScrollCanvas& canvas, const float scroll_factor);

	void setScrollPos(const int x, const int y); // Sets SAME FOR ALL
	void moveScrollBy(const int dx, const int dy); // Parallax, assumes zero start
	void setScrollSpeed(const float speed_x, const float speed_y); // In px per sec, parallax.
	void setScrollStart(const int x, const int y); // Sets SAME FOR ALL

	void resetScrollToStart();
	
	void pauseScroll();
	void resumeScroll();

	void event(BlockEvent e) override;
	void action(BlockAction a) override;
	void draw() override;

protected:
	void onResize() override;

	struct SCFactor {
		SCFactor(ScrollCanvas& sc, const float factor) : canvas(&sc), scroll_factor(factor) {}

		ScrollCanvas* canvas = nullptr;
		float scroll_factor = 1.0;
	};

	// Drawn in order, only first is evented (other than scroll events).
	std::vector<SCFactor> canvases_;

	DELETE_COPY_AND_ASSIGN(ParallaxScrollGroup);
};

} // namespace Blocks
