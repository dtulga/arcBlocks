#pragma once

#include <vector>

#include "block.h"

namespace Blocks {

class Canvas : public ExpandingInteractive {
public:
	struct Properties {
		Color background_color; // Default is transparent (i.e. no background color)
	};

	struct BlockElement {
		Drawable* d = nullptr; // NEVER null
		ExpandingDrawable* ed = nullptr; // Not-null only if expanding.
		ScreenEventable* se = nullptr; // Not-null only if interactive.
		
		// Requested coordinates:
		// + is from top/left and - is from bottom/right
		int x;
		int y;

		// Current draw coordinates. (auto-calculated)
		int draw_x;
		int draw_y;
	};

	Canvas(Properties props, ExpandingProperties ex_props) : ExpandingInteractive(ex_props), props_(props) {}
	~Canvas();

	// Positive Coordinates are from top/left and negative are from bottom/right (auto-resizing)
	BlockElement* addDrawableAt(Drawable& block, const int x, const int y);
	BlockElement* addExpandingDrawableAt(ExpandingDrawable& block, const int x, const int y);
	BlockElement* addInteractiveAt(Interactive& block, const int x, const int y);
	BlockElement* addExpandingInteractiveAt(ExpandingInteractive& block, const int x, const int y);
	// All drawn in order of addition, and evented in reverse order (So first is background, then topmost interactive should be last)

	void removeBlock(Drawable& block); // Works for all types.
	void removeBlock(BlockElement* block_element);
	
	void event(BlockEvent e) override;
	void action(BlockAction a) override;
	void draw() override;

	static const constexpr int Centered = AlignCentered;

protected:
	void onResize() override;

	bool SendScreenEventHitTestAll(std::vector<BlockElement*>& int_blocks, const BlockEvent& e); // Tests all interactives for the given screen event.
	bool SendScreenEventHitTest(BlockEvent e, BlockElement* be); // Also transforms the local coordinates, and returns true if sent, false if not.

	void sendCanvasEvent(BlockEvent e);

	void AssignDrawCoords(BlockElement* be);

	Properties props_;
	
	// All blocks which this canvas directly draws/aligns/and passes screen events to.
	// Owned by this vector:
	std::vector<BlockElement*> drawables_;
	// Subset of the above, for fast screen event processing: (Not Owned)
	std::vector<BlockElement*> interactives_;
};

} // namespace Blocks
