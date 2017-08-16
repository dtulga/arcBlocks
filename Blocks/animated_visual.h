#pragma once

#include "block.h"

namespace Blocks {

class AnimatedVisual : public Drawable {
public:
	struct Properties {
		Sprite* sprite = nullptr; // This holds all the animation frames.

		// This can also be set for a background color drawn first (optional).
		Color background_color;

		// Animation properties
		int start_offset_x = 0;
		int start_offset_y = 0;
		int frame_spacing_x = 0; // Start of frame to next start of frame (x)
		int frame_spacing_y = 0; // Start of frame to next start of frame (y)
		uint32_t n_frames_total = 1;
		// TODO: Time-based duration //
		uint32_t animation_frame_duration = 1; // Set to >1 for multiple actual frames per animation frame (1 = 60, 2 = 30, 3 = 20, 4 = 15, 5 = 12, etc.)
		bool back_and_forth = false; // Otherwise goes from the last to the first frame

		// TODO: Transform for stretching/scaling/etc. + recolor sprites, etc.
	};

	explicit AnimatedVisual(const Properties& props, const int width, const int height)
	 : Drawable(width, height), props_(props) {}

	void setSprite(Sprite& s);
	void setBackgroundColor(const Color& bg_color);

	// To use multiple animations in one sprite
	void setStartOffset(const int off_x, const int off_y) { props_.start_offset_x = off_x; props_.start_offset_y = off_y; }

	void pause() { playing_ = false; }
	void play() { playing_ = true; }

	void draw() override;

protected:
	Properties props_;

	uint32_t frame_ = 0;
	uint32_t frame_skip_ = 0; // For animation_frame_duration > 1
	uint32_t last_manager_frame_count_ = 0;

	bool playing_ = true;
};
	
} // namespace Blocks
