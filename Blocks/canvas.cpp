#include "canvas.h"

namespace Blocks {

Canvas::~Canvas() {
	deleteallslotvector(drawables_);
}

// Positive Coordinates are from top/left and negative are from bottom/right (auto-resizing)
Canvas::BlockElement* Canvas::addDrawableAt(Drawable& block, const int x, const int y) {
	BlockElement* be = new BlockElement();
	be->d = &block;
	be->x = x;
	be->y = y;
	AssignDrawCoords(be);
	addtoslotvectorindex(drawables_, be);
	return be;
}

Canvas::BlockElement* Canvas::addExpandingDrawableAt(ExpandingDrawable& block, const int x, const int y) {
	BlockElement* be = new BlockElement();
	be->d = &block;
	be->ed = &block;
	be->x = x;
	be->y = y;
	AssignDrawCoords(be);
	addtoslotvectorindex(drawables_, be);
	return be;
}

Canvas::BlockElement* Canvas::addInteractiveAt(Interactive& block, const int x, const int y) {
	BlockElement* be = new BlockElement();
	be->d = &block;
	be->se = &block;
	be->x = x;
	be->y = y;
	AssignDrawCoords(be);
	addtoslotvectorindex(drawables_, be);
	addtoslotvectorindex(interactives_, be);
	return be;
}

Canvas::BlockElement* Canvas::addExpandingInteractiveAt(ExpandingInteractive& block, const int x, const int y) {
	BlockElement* be = new BlockElement();
	be->d = &block;
	be->ed = &block;
	be->se = &block;
	be->x = x;
	be->y = y;
	AssignDrawCoords(be);
	addtoslotvectorindex(drawables_, be);
	addtoslotvectorindex(interactives_, be);
	return be;
}

void Canvas::removeBlock(Drawable& block) { // Works for all types.
	Drawable* d = &block;

	const size_t len = drawables_.size();
	for (size_t i = 0; i < len; i++) {
		BlockElement* be = drawables_[i];
		if (be != nullptr && be->d == d) {
			drawables_[i] = nullptr; // Remove the block from the list, but don't delete it.
			if (be->se != nullptr) { // Also remove from the interactives_ list.
				for (size_t j = 0; j < interactives_.size(); j++) {
					if (interactives_[j] == be) {
						interactives_[j] = nullptr;
						break;
					}
				}
			}
			delete be; // Don't need the block element anymore, although the block itself is not deleted.
			return;
		}
	}
}

void Canvas::removeBlock(BlockElement* block_element) {
	if (block_element == nullptr) return;

	const size_t len = drawables_.size();
	for (size_t i = 0; i < len; i++) {
		BlockElement* be = drawables_[i];
		if (be != nullptr && be == block_element) {
			drawables_[i] = nullptr; // Remove the block from the list, but don't delete it.
			if (be->se != nullptr) { // Also remove from the interactives_ list.
				for (size_t j = 0; j < interactives_.size(); j++) {
					if (interactives_[j] == be) {
						interactives_[j] = nullptr;
						break;
					}
				}
			}
			delete be; // Don't need the block element anymore, although the block itself is not deleted.
			return;
		}
	}
}

bool Canvas::SendScreenEventHitTestAll(std::vector<BlockElement*>& int_blocks, const BlockEvent& e) {
	for (size_t i = int_blocks.size(); i > 0; i--) {
		BlockElement* be = int_blocks[i - 1];
		if (be == nullptr) continue;
		if (SendScreenEventHitTest(e, be)) { // false on nullptr
			return true;
		}
	}
	return false;
}

void Canvas::sendCanvasEvent(BlockEvent e) {
	switch (e.type) {
	case BlockEvent::PressDown:
		e.type = BlockEvent::CanvasPressDown;
		sendEvent(e);
		break;
	case BlockEvent::PressUp:
		e.type = BlockEvent::CanvasPressUp;
		sendEvent(e);
		break;
	case BlockEvent::PressDrag:
		e.type = BlockEvent::CanvasDrag;
		sendEvent(e);
		break;
		// TODO: DragOut, etc. //
		// All others ignore for now.
	}
}

void Canvas::event(BlockEvent e) {
	if (e.is_screen()) {
		if (SendScreenEventHitTestAll(interactives_, e)) return; // Event matched, sent.

		// Event didn't match any interactives, may send as a canvas event:
		sendCanvasEvent(e);
	}
	// TODO: Other events?
}

void Canvas::action(BlockAction a) {
	if (a.is_movement()) {
		BlockElement* be = (BlockElement*) a.data;
		if (be == nullptr) {
			log::Error("Canvas", "Movement action received with null data!"); return;
		}
		// TODO: Check that this block is actually in this canvas?
		Drawable* d = be->d;
		if (a.type == BlockAction::SetPos) {
			be->x = min_max(0, a.position.x, width_ - d->width());
			be->y = min_max(0, a.position.y, height_ - d->height());
			AssignDrawCoords(be);
		} else if (a.type == BlockAction::MoveBy) {
			be->x = min_max(0, be->x + a.delta.dx, width_ - d->width());
			be->y = min_max(0, be->y + a.delta.dy, height_ - d->height());
			AssignDrawCoords(be);
		}
	}
}

// TODO: HoverEnter/Exit + Drag
bool Canvas::SendScreenEventHitTest(BlockEvent e, BlockElement* be) {
	if (be == nullptr) return false;

	ScreenEventable* se = be->se;
	if (!se->is_enabled()) return false;

	// Note that these are local coordinates (to this canvas).
	int x, y, w, h;
	const int ex = e.screen.x;
	const int ey = e.screen.y;
	
	se->getEventRegion(x, y, w, h);
	x += be->draw_x;
	y += be->draw_y;
	if (ex >= x && ey >= y && ex <= x + w && ey <= y + h) {
		// Transform to local coordinates for the block.
		e.screen.x -= be->draw_x;
		e.screen.y -= be->draw_y;
		se->event(e);
		return true;
	}
	return false;
}

// This resizes all blocks that are expanding + re-assigns the draw coordinates.
void Canvas::onResize() {
	const size_t len = drawables_.size();
	for (size_t i = 0; i < len; i++) {
		BlockElement* be = drawables_[i];
		if (be == nullptr) continue;
		AssignDrawCoords(be);
		if (be->ed != nullptr) {
			be->ed->setDrawableArea(width_ - be->draw_x, height_ - be->draw_y);
		}
	}
	BlockEvent e(BlockEvent::CanvasResize);
	e.screen.x = width_;
	e.screen.y = height_;
	// TODO: dx/dy in this event //
	sendEvent(e);
}

void Canvas::AssignDrawCoords(BlockElement* be) {
	if (be == nullptr) return;

	int x, y;

	if (be->x == Centered) {
		x = (width_ / 2) - (be->d->width() / 2);
	} else if (be->x >= 0) {
		x = be->x;
	} else {
		x = width_ + be->x; // Negative x
	}

	if (be->y == Centered) {
		y = (height_ / 2) - (be->d->height() / 2);
	} else if (be->y >= 0) {
		y = be->y;
	} else {
		y = height_ + be->y; // Negative y
	}

	be->draw_x = x;
	be->draw_y = y;
}

void Canvas::draw() {
	int off_x, off_y;
	render.GetDrawOffset(off_x, off_y); // In case this is a sub-canvas.

	if (!props_.background_color.is_transparent()) {
		render.DrawRect(width_, height_, props_.background_color);
	}

	const size_t len = drawables_.size();
	for (size_t i = 0; i < len; i++) {
		BlockElement* be = drawables_[i];
		if (be == nullptr) continue;

		Drawable* d = be->d;
		if (d->is_visible()) {
			render.SetDrawOffset(off_x + be->draw_x, off_y + be->draw_y); // TODO: Clipping off of canvas boundaries?
			d->draw(); // Drawable::draw() - virtual function should resolve the correct call.
		}
	}
}

} // namespace Blocks
