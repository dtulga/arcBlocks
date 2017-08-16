#pragma once

#include "block.h"

namespace Blocks {

class CompoundVisual : public Drawable {
public:
	struct Properties {
		// This can also be set for a background color drawn first (optional).
		Color background_color;

		// Other visuals to draw
		std::vector<BasicAlignment> visuals;
	};

	CompoundVisual(const Properties& props, const int width, const int height)
		: Drawable(width, height), props_(props) {}

	void setBackgroundColor(const Color& bg_color) { props_.background_color = bg_color; }

	void addAlignedDrawable(const BasicAlignment& a) { props_.visuals.push_back(a); }
	void addAlignedDrawable(BasicAlignment&& a) { props_.visuals.push_back(std::move(a)); }

	void addDrawableAt(Drawable* d, const int xpos, const int ypos) { props_.visuals.push_back(BasicAlignment(d, xpos, ypos)); }

	void draw() override;

protected:
	Properties props_;
};

} // namespace Blocks
