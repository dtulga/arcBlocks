#include "button.h"

namespace Blocks {

void Button::event(BlockEvent e) {
	if (!e.is_screen()) return;

	int fx = 0;
	int fy = 0;

	switch (e.type) {
		case BlockEvent::PressDown:
		if (state_ == StateDefault || state_ == StateHover) {
			state_ = StateClick;
			pressed_down_ = true;
			e.type = BlockEvent::ButtonPressDown;
			sendEvent(e);
		}
		break;
		case BlockEvent::PressUp:
		if (state_ == StateClick) {
			if (hovered_) {
				state_ = StateHover;
			} else {
				state_ = StateDefault;
			}
			// Send Event
			if (pressed_down_) {
				pressed_down_ = false;
				e.type = BlockEvent::ButtonPressed;
				sendEvent(e);
			}
		}
		break;
		case BlockEvent::PressDrag:
		e.type = BlockEvent::ButtonDrag;
		sendEvent(e); // Drag event.

		fx = e.screen.x + e.screen.dx;
		fy = e.screen.y + e.screen.dy;

		if (fx >= width_ || fx < 0 || fy >= height_ || fy < 0) {
			pressed_down_ = false;
			e.type = BlockEvent::ButtonDragOut; // Also send a DragOut event.
			sendEvent(e);
		}
		break;
		case BlockEvent::HoverEnter:
		case BlockEvent::Hover:
		hovered_ = true;
		if (state_ == StateDefault) {
			state_ = StateHover;
		}
		break;
		case BlockEvent::HoverExit:
		hovered_ = false;
		if (state_ == StateHover) {
			state_ = StateDefault;
		}
	}
}

// Actions: Enable, Disable, etc.
void Button::action(BlockAction a) {
	if (a.type == BlockAction::ButtonEnable && state_ == StateDisabled) {
		state_ = StateDefault;
	} else if (a.type == BlockAction::ButtonDisable) {
		state_ = StateDisabled;
	} else if (a.type == BlockAction::ButtonToggleEnabled) {
		if (state_ == StateDisabled) {
			state_ = StateDefault;
		} else {
			state_ = StateDisabled;
		}
	}
}

void Button::draw() {
	switch (state_) {
		case StateHover:
		if (props_.hover_v != nullptr) {
			props_.hover_v->draw();
			return;
		}
		break;
		case StateClick:
		if (props_.click_v != nullptr) {
			props_.click_v->draw();
			return;
		}
		break;
		case StateDisabled:
		if (props_.disabled_v != nullptr) {
			props_.disabled_v->draw();
			return;
		}
		break;
		case StateDefault:
		default:
		break;
	}
	props_.default_v->draw();
}

} // namespace Blocks
