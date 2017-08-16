#include "scroll_canvas.h"

namespace Blocks {

Canvas::BlockElement* ScrollCanvas::addScrollDrawableAt(Drawable& block, const int x, const int y) {
	BlockElement* be = new BlockElement();
	be->d = &block;
	be->x = x;
	be->y = y;
	addtoslotvectorindex(sc_drawables_, be);
	return be;
}

Canvas::BlockElement* ScrollCanvas::addScrollInteractiveAt(Interactive& block, const int x, const int y) {
	BlockElement* be = new BlockElement();
	be->d = &block;
	be->se = &block;
	be->x = x;
	be->y = y;
	addtoslotvectorindex(sc_drawables_, be);
	addtoslotvectorindex(sc_interactives_, be);
	return be;
}

void ScrollCanvas::removeScrollBlock(Drawable& block) { // Works for all types.
	Drawable* d = &block;

	const size_t len = sc_drawables_.size();
	for (size_t i = 0; i < len; i++) {
		BlockElement* be = sc_drawables_[i];
		if (be != nullptr && be->d == d) {
			sc_drawables_[i] = nullptr; // Remove the block from the list, but don't delete it.
			if (be->se != nullptr) { // Also remove from the interactives_ list.
				for (size_t j = 0; j < sc_interactives_.size(); j++) {
					if (sc_interactives_[j] == be) {
						sc_interactives_[j] = nullptr;
						break;
					}
				}
			}
			delete be; // Don't need the block element anymore, although the block itself is not deleted.
			return;
		}
	}
}

void ScrollCanvas::removeScrollBlock(BlockElement* block_element) {
	if (block_element == nullptr) return;

	const size_t len = sc_drawables_.size();
	for (size_t i = 0; i < len; i++) {
		BlockElement* be = sc_drawables_[i];
		if (be != nullptr && be == block_element) {
			sc_drawables_[i] = nullptr; // Remove the block from the list, but don't delete it.
			if (be->se != nullptr) { // Also remove from the interactives_ list.
				for (size_t j = 0; j < sc_interactives_.size(); j++) {
					if (sc_interactives_[j] == be) {
						sc_interactives_[j] = nullptr;
						break;
					}
				}
			}
			delete be; // Don't need the block element anymore, although the block itself is not deleted.
			return;
		}
	}
}

void ScrollCanvas::setScrollPos(const int x, const int y) {
	sc_x_ = x;
	sc_y_ = y;
	rem_x_ = 0.0f;
	rem_y_ = 0.0f;

	CalcWrapValues();
}

void ScrollCanvas::moveScrollBy(const int dx, const int dy) {
	if (dx != 0) {
		const int min_x = sc_props_.min_x;
		const int max_x = sc_props_.max_x;
		if (sc_props_.wrap_x == ScrollProperties::WRAP_TOROIDAL) {
			sc_x_ += dx;
			if (sc_x_ < min_x) {
				sc_x_ = max_x - (min_x - sc_x_);
			} else if (sc_x_ > max_x) {
				sc_x_ = min_x + (sc_x_ - max_x);
			}
		} else { // WRAP_NONE
			sc_x_ = min_max(min_x, sc_x_ + dx, max_x);
		}
	}

	if (dy != 0) {
		const int min_y = sc_props_.min_y;
		const int max_y = sc_props_.max_y;
		if (sc_props_.wrap_y == ScrollProperties::WRAP_TOROIDAL) {
			sc_y_ += dy;
			if (sc_y_ < min_y) {
				sc_y_ = max_y - (min_y - sc_y_);
			} else if (sc_y_ > max_y) {
				sc_y_ = min_y + (sc_y_ - max_y);
			}
		} else { // WRAP_NONE
			sc_y_ = min_max(min_y, sc_y_ + dy, max_y);
		}
	}

	CalcWrapValues();
}

void ScrollCanvas::setScrollSpeed(const float speed_x, const float speed_y) {
	sc_props_.speed_x = speed_x;
	sc_props_.speed_y = speed_y;
	rem_x_ = 0.0f;
	rem_y_ = 0.0f;
}

void ScrollCanvas::setScrollStart(const int x, const int y) {
	sc_props_.start_x = x;
	sc_props_.start_y = y;
}

void ScrollCanvas::resetScrollToStart() {
	sc_x_ = sc_props_.start_x;
	sc_y_ = sc_props_.start_y;
	rem_x_ = 0.0f;
	rem_y_ = 0.0f;

	if (sc_props_.wrap_x == ScrollProperties::WRAP_TOROIDAL) {
		wrap_total_x_ = sc_props_.max_x - sc_props_.min_x;
	} else {
		wrap_total_x_ = 0;
	}

	if (sc_props_.wrap_y == ScrollProperties::WRAP_TOROIDAL) {
		wrap_total_y_ = sc_props_.max_y - sc_props_.min_y;
	} else {
		wrap_total_y_ = 0;
	}

	CalcWrapValues();
}

void ScrollCanvas::event(BlockEvent e) {
	if (e.is_screen()) {
		// First, test all normal (top-layer) canvas elements:
		if (Canvas::SendScreenEventHitTestAll(interactives_, e)) return;

		// Then, check scrolling elements: (draw_x / draw_y are canvas coords, so this works.)
		if (Canvas::SendScreenEventHitTestAll(sc_interactives_, e)) return;

		// Then send as a canvas event if none others match:
		Canvas::sendCanvasEvent(e);
	}

	// TODO: Any other events?
}

void ScrollCanvas::action(BlockAction a) {
	if (a.is_scroll()) {
		switch (a.type) {
		case BlockAction::SetScrollPos:
			setScrollPos(a.position.x, a.position.y);
			break;
		case BlockAction::MoveScrollBy:
			moveScrollBy(a.delta.dx, a.delta.dy);
			break;
		case BlockAction::ResetScroll:
			resetScrollToStart();
			break;
		case BlockAction::PauseScroll:
			pauseScroll();
			break;
		case BlockAction::ResumeScroll:
			resumeScroll();
			break;
		default:
			log::Error("ScrollCanvas", "Unhandled scroll event type!");
			break;
		}
	} else if (a.is_movement()) {
		BlockElement* be = (BlockElement*)a.data;
		if (be == nullptr) {
			log::Error("ScrollCanvas", "Movement action received with null data!"); return;
		}

		// TODO: Check that this block is actually in this canvas + is a scroll element?
		if (a.type == BlockAction::SetPos) {
			// Note no boundary checks as elements can totally be off-screen
			be->x = a.position.x;
			be->y = a.position.y;
		} else if (a.type == BlockAction::MoveBy) {
			// Note no boundary checks as elements can totally be off-screen
			be->x += a.delta.dx;
			be->y += a.delta.dy;
			AssignDrawCoords(be);
		}
	} else {
		Canvas::action(a);
	}
}

void ScrollCanvas::CalcWrapValues() {
	wrap_x_max_ = sc_props_.wrap_x == ScrollProperties::WRAP_TOROIDAL &&
		((sc_props_.max_x - sc_x_) < width_);
	wrap_y_max_ = sc_props_.wrap_y == ScrollProperties::WRAP_TOROIDAL &&
		((sc_props_.max_y - sc_y_) < height_);
}

void ScrollCanvas::GetScrollCoords(BlockElement* be, const Drawable* d, int& x, int& y, int& xw, int& yh) {
	if (be == nullptr) return;

	x = be->x - sc_x_;
	y = be->y - sc_y_;
	xw = x + d->width();
	yh = y + d->height();

	// TODO: Other wrapping types.
	if (wrap_x_max_ && xw < 0) { // Right wrap visible
		x += wrap_total_x_;
		xw += wrap_total_x_;
	}
	if (wrap_y_max_ && yh < 0) { // Bottom wrap visible
		y += wrap_total_y_;
		yh += wrap_total_y_;
	}
	// Note the top/left or negative wraps never occur, as it auto-sets the scroll to near the max.

	// Canvas-relative coordinates, used for the event test:
	be->draw_x = x;
	be->draw_y = y;
}

void ScrollCanvas::draw() {
	const size_t len = sc_drawables_.size();

	int off_x, off_y, x, y, xw, yh;
	render.GetDrawOffset(off_x, off_y); // In case this is a sub-canvas.

	for (size_t i = 0; i < len; i++) {
		BlockElement* be = sc_drawables_[i];
		if (be == nullptr) continue;

		Drawable* d = be->d;
		if (!d->is_visible()) continue;

		// Get scrolled to actual coordinates
		GetScrollCoords(be, d, x, y, xw, yh);

		// Draw if at least partially on-screen.
		if (xw > 0 && yh > 0 && x < width_ && y < height_) {
			render.SetDrawOffset(off_x + x, off_y + y); // TODO: Clipping off of canvas boundaries?
			d->draw();
		}
	}

	// TODO: Background color can overlap here!
	Canvas::draw(); // Draw all non-scrolling objects (above on the top layer).
}

void ScrollCanvas::frame(double frame_delta_time_msec) {
	if (!scrolling_) return;

	const float delta_msec = (float)frame_delta_time_msec;

	int dx = 0;
	if (sc_props_.type_x == ScrollProperties::AUTO) {
		const float sp_x = sc_props_.speed_x;

		if (sp_x != 0.0) {
			rem_x_ += (delta_msec * sp_x) / 1000.0f;
			dx = (int)rem_x_;
			if (dx != 0) {
				rem_x_ -= (float)dx;
			}
		}
	}

	int dy = 0;
	if (sc_props_.type_y == ScrollProperties::AUTO) {
		const float sp_y = sc_props_.speed_y;

		if (sp_y != 0.0) {
			rem_y_ += (delta_msec * sp_y) / 1000.0f;
			dy = (int)rem_y_;
			if (dy != 0) {
				rem_y_ -= (float)dy;
			}
		}
	}

	moveScrollBy(dx, dy);

	// TODO: Other scroll types //
}

} // namespace Blocks
