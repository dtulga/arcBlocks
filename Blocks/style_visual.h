#pragma once

#include "block.h"

namespace Blocks {

class StyleVisual : public Drawable {
public:
	struct Properties {
		// This can also be set for a background color drawn first (optional).
		Color background_color;
		Color border_color;

		int border_thickness = 0;

		int padding_left = 0;
		int padding_right = 0;
		int padding_top = 0;
		int padding_bottom = 0;
		
		int margin_left = 0;
		int margin_right = 0;
		int margin_top = 0;
		int margin_bottom = 0;

		void setPadding(const int padding) {
			padding_left = padding;
			padding_right = padding;
			padding_top = padding;
			padding_bottom = padding;
		}

		void setMargin(const int margin) {
			margin_left = margin;
			margin_right = margin;
			margin_top = margin;
			margin_bottom = margin;
		}
	};

	StyleVisual(const Properties& props, Drawable& visual)
		: Drawable(
			visual.width() + (props.border_thickness * 2) + props.padding_left + props.padding_right + props.margin_left + props.margin_right,
			visual.height() + (props.border_thickness * 2) + props.padding_top + props.padding_bottom + props.margin_top + props.margin_bottom),
		  props_(props), visual_(&visual) {}

	void setBackgroundColor(const Color& bg_color) { props_.background_color = bg_color; }
	void setBorderColor(const Color& border_color) { props_.border_color = border_color; }

	void setVisual(Drawable& visual) { visual_ = &visual; }

	void draw() override;

protected:
	Properties props_;
	Drawable* visual_ = nullptr;
};

} // namespace Blocks
