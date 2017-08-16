#pragma once

#include "block.h"
#include "variable.h"

namespace Blocks {

class VarVisual : public Drawable {
public:
	struct Properties {
		// NOT Owned:
		IntVariable* int_var = nullptr;
		// TODO: Add other var types/replace with arc::var when ready.

		size_t pixel_font_id = -1; // Must be set!

		// This can also be set for a background color drawn first.
		Color background_color;

		// TODO: Auto-set these?
		int width = 0;
		int height = 0;

		// Text padding
		uint32_t pad_to_length = 0;
		uint8_t pad_with_char = ' ';
		//bool pad_at_beginning = true; // Otherwise padding goes at the end.
	};

	explicit VarVisual(const Properties& props)
		: Drawable(props.width, props.height), props_(props) {

		if (props.pad_to_length > 0) {
			buffer_.reserve(props.pad_to_length);
		}
	}

	void setIntVariable(IntVariable& int_var) { props_.int_var = &int_var; }
	void setBackgroundColor(const Color& bg_color) { props_.background_color = bg_color; }

	void draw() override;

protected:
	Properties props_;

	string buffer_;
};

} // namespace Blocks
