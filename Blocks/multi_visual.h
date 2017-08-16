#pragma once

#include <vector>

#include "block.h"

namespace Blocks {

struct MultiVisualStep {
	MultiVisualStep(Drawable& vis) : visual(&vis) {}
	MultiVisualStep(Drawable& vis, const uint32_t frames) : visual(&vis), n_frames(frames) {}

	// NOT Owned:
	Drawable* visual = nullptr; // Can be null to have an empty visual as a step.
	uint32_t n_frames = 0; // Use 0 for indefinite
};

// Multiple steps, each step with a time/frame duration, can be any drawable, animated, etc.
// Sends events when steps are finished, etc.
class MultiVisual : public Interactive {
public:
	struct Properties {
		std::vector<MultiVisualStep> steps;

		// Drawing width/height
		int width = 0;
		int height = 0;

		// Event region/hitbox
		int event_x = 0;
		int event_y = 0;
		// These default to above if set to zero here + auto_event_region = true
		int event_w = 0;
		int event_h = 0;

		bool auto_event_region = true;
		bool auto_center_steps = false;
		bool restart_after_last_step = false;
	};

	MultiVisual(const Properties& props) : Interactive(props.width, props.height), props_(props) {
		if (props.auto_event_region) {
			props_.event_w = props.width;
			props_.event_h = props.height;
		}
	}
	~MultiVisual() {}

	void getEventRegion(int& x, int& y, int& width, int& height) override {
		const Properties& p = props_;
		x = p.event_x;
		y = p.event_y;
		width = p.event_w;
		height = p.event_h;
	}

	void nextStep();
	void prevStep();
	void lastStep();
	void firstStep();
	void setStep(const size_t step_id);
	size_t curStep() const { return cur_step_; }

	void event(BlockEvent e) override;
	void action(BlockAction a) override;

	void draw() override;

protected:
	Properties props_;

	size_t cur_step_ = 0;
	size_t cur_step_frames_ = 0; // Number of frames of this step that have been drawn already.
	uint32_t last_manager_frame_count_ = 0; // To prevent duplicate frame updates.
};

} // namespace Blocks
