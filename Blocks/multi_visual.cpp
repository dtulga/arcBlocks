#include "multi_visual.h"

#include "manager.h"

namespace Blocks {

void MultiVisual::nextStep() {
	if (cur_step_ + 1 >= props_.steps.size()) {
		if (props_.restart_after_last_step) {
			cur_step_ = 0;
			cur_step_frames_ = 0;
		}
		// TODO: Print Error?
	} else {
		cur_step_++;
		cur_step_frames_ = 0;
	}
}

void MultiVisual::prevStep() {
	if (cur_step_ > 0) {
		cur_step_--;
		cur_step_frames_ = 0;
	}
	// TODO: Print Error?
}

void MultiVisual::lastStep() {
	cur_step_ = props_.steps.size() - 1;
	cur_step_frames_ = 0;
}

void MultiVisual::firstStep() {
	cur_step_ = 0;
	cur_step_frames_ = 0;
}

void MultiVisual::setStep(const size_t step_id) {
	if (step_id < props_.steps.size()) {
		cur_step_ = step_id;
		cur_step_frames_ = 0;
	}
	// TODO: Print Error?
}

void MultiVisual::event(BlockEvent e) {
	if (!e.is_screen()) return;

	int fx = 0;
	int fy = 0;

	switch (e.type) {
	case BlockEvent::PressDown:
		e.type = BlockEvent::VisualPressDown;
		sendEvent(e);
		break;
	case BlockEvent::PressUp:
		e.type = BlockEvent::VisualPressUp;
		sendEvent(e);
		break;
	case BlockEvent::PressDrag:
		e.type = BlockEvent::VisualDrag;
		sendEvent(e); // Drag event.

		fx = e.screen.x + e.screen.dx;
		fy = e.screen.y + e.screen.dy;

		if (fx >= width_ || fx < 0 || fy >= height_ || fy < 0) {
			e.type = BlockEvent::VisualDragOut; // Also send a DragOut event.
			sendEvent(e);
		}
		break;
	}
}

void MultiVisual::action(BlockAction a) {
	if (!a.is_step()) return;

	switch (a.type) {
	case BlockAction::SetStep:
		setStep(a.id);
		break;
	case BlockAction::NextStep:
		nextStep();
		break;
	case BlockAction::PrevStep:
		prevStep();
		break;
	case BlockAction::FirstStep:
		firstStep();
		break;
	case BlockAction::LastStep:
		lastStep();
		break;
	default:
		log::Error("MultiVisual", "Unhandled step event type!");
		break;
	}
}

void MultiVisual::draw() {
	const Properties& p = props_;
	const MultiVisualStep& step = p.steps[cur_step_];
	Drawable* d = step.visual;

	if (d != nullptr) {
		if (p.auto_center_steps && (d->width() != width_ || d->height() != height_)) {
			render.AddDrawOffset((width_ / 2) - (d->width() / 2), (height_ / 2) - (d->height() / 2));
		}
		d->draw();
	}

	const uint32_t mfc = manager.GetFrameCount();
	if (last_manager_frame_count_ == mfc) {
		return; // Already updated this frame.
	} else {
		last_manager_frame_count_ = mfc;
	}

	const uint32_t max_frames = step.n_frames;

	if (max_frames != 0) {
		cur_step_frames_++;

		if (cur_step_frames_ >= max_frames) {
			nextStep();
		}
	}
}

} // namespace Blocks
