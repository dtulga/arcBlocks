#include "animated_visual.h"
#include "manager.h"

namespace Blocks {

// TODO: Error checking if ondraw is already set? //
void AnimatedVisual::setSprite(Sprite& s) {
	props_.sprite = &s; // Must have the same animation properties! // TODO: Allow changing properties? //
}

void AnimatedVisual::setBackgroundColor(const Color& bg_color) {
	props_.background_color = bg_color;
}

void AnimatedVisual::draw() {
	if (!props_.background_color.is_transparent()) {
		render.DrawRect(width_, height_, props_.background_color);
	}

	if (props_.sprite == nullptr) {
		log::Error("AnimatedVisual", "Attempted to draw a null animated visual");
		return;
	}

	const uint32_t n = props_.n_frames_total;
	uint32_t f = frame_;

	if (props_.back_and_forth) {
		if (f >= n) {
			f = (2 * n) - 1 - frame_;
		}
	}

	int cur_x = props_.start_offset_x + (f * props_.frame_spacing_x);
	int cur_y = props_.start_offset_y + (f * props_.frame_spacing_y);
	render.DrawSpriteSubRegion(*props_.sprite, 0, 0, cur_x, cur_y, width_, height_, true); // Allow cropping

	// Advance frame
	if (!playing_) return;

	const uint32_t mfc = manager.GetFrameCount();
	if (last_manager_frame_count_ == mfc) {
		return; // Already updated this frame.
	} else {
		last_manager_frame_count_ = mfc;
	}

	if (props_.animation_frame_duration > 1) { // TODO: Make this time-based? //
		frame_skip_++;
		if (frame_skip_ < props_.animation_frame_duration) {
			return;
		}
		frame_skip_ = 0; // Actually advance the frame
	}
	frame_++;
	if (props_.back_and_forth) {
		if (frame_ >= 2 * n) {
			frame_ = 0;
		}
	} else {
		if (frame_ >= n) {
			frame_ = 0;
		}
	}
}

} // namespace Blocks
