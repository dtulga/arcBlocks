#pragma once

#include "block.h"

namespace Blocks {

class Visual : public Drawable {
public:
	struct Properties {
		// SET ONLY ONE OF THESE: + All NOT Owned:
		void (*ondraw_func)() = nullptr;
		Sprite* sprite = nullptr;
		string pixel_text;
		size_t pixel_font_id = -1; // Must be set for pixel_text

		// This can also be set for a background color drawn first. (Useful for rectangles, etc. as well)
		Color background_color;

		// TODO: Transform for stretching/scaling/etc. + recolor sprites, etc.
	};

	// Single sprite
	explicit Visual(Sprite& sprite) : Drawable(sprite.width(), sprite.height()) {
		props_.sprite = &sprite;
	}

	// Solid/Background color
	Visual(const Color& color, const int width, const int height) : Drawable(width, height) {
		props_.background_color = color;
	}

	// Dynamic ondraw function
	Visual(void (*ondraw_func)(), const int width, const int height) : Drawable(width, height) {
		props_.ondraw_func = ondraw_func;
		// TODO: Allow for a draw-once function here?
	}

	// Static pixel text
	Visual(const string& pixel_text, const size_t pixel_font_id = 0)
		: Drawable(render.GetPixelTextWidth(pixel_text, pixel_font_id),
				   render.GetPixelTextHeight(pixel_text, pixel_font_id)) {
		props_.pixel_text = pixel_text;
		props_.pixel_font_id = pixel_font_id;
	}

	// MUST set width/height for an ondraw_func or plain background color, otherwise auto-detects from the sprite.
	explicit Visual(const Properties& props, const int width = 0, const int height = 0)
	 : Drawable(width == 0 ? props.sprite->width() : width, height == 0 ? props.sprite->height() : height), props_(props) {}

	void setSprite(Sprite& s);
	void setBackgroundColor(const Color& bg_color);
	void setPixelText(const string& pixel_text) { props_.pixel_text = pixel_text; }
	
	void draw() override;

protected:
	Properties props_;
};

inline void Visual::setBackgroundColor(const Color& bg_color) {
	props_.background_color = bg_color;
}
	
} // namespace Blocks
