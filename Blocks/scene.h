#pragma once

#include "canvas.h"

namespace Blocks {

// Handler for multiple main canvases, one per scene.
class Scene : public ExpandingInteractive {
public:
	Scene(Canvas& first_scene_canvas, const ExpandingProperties& ex_props) : ExpandingInteractive(ex_props) {
		scenes_.push_back(&first_scene_canvas);
	}

	size_t addScene(Canvas& scene_canvas) { scenes_.push_back(&scene_canvas); return (scenes_.size() - 1); }

	void setScene(const size_t id);

	// Useful for backgrounds, etc.
	void addDrawableToAllScenesAt(Drawable& block, const int x, const int y);
	void addExpandingDrawableToAllScenesAt(ExpandingDrawable& block, const int x, const int y);

	void event(BlockEvent e) override;
	void action(BlockAction a) override;
	void draw() override;

protected:
	void onResize() override;

	// NOT Owned:
	std::vector<Canvas*> scenes_;

	size_t cur_scene_ = 0;
};

} // namespace Blocks
