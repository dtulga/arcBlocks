#include "parallax_scroll.h"

#define PS_FOR_ALL_CANVASES(func) const size_t len = canvases_.size(); \
for (size_t i = 0; i < len; i++) { \
	ScrollCanvas* sc = canvases_[i].canvas; \
	if (sc != nullptr) { \
		sc->func(); \
	} \
}

#define PS_FOR_ALL_CANVASES_REVERSE(func) const size_t len = canvases_.size(); \
for (size_t i = len; i > 0; i--) { \
	ScrollCanvas* sc = canvases_[i - 1].canvas; \
	if (sc != nullptr) { \
		sc->func(); \
	} \
}

#define PS_FOR_ALL_CANVASES_2(func, arg1, arg2) const size_t len = canvases_.size(); \
for (size_t i = 0; i < len; i++) { \
	ScrollCanvas* sc = canvases_[i].canvas; \
	if (sc != nullptr) { \
		sc->func(arg1, arg2); \
	} \
}

#define PS_ADJUST_FLOAT(value) int(value * canvases_[i].scroll_factor)

#define PS_ADJUST_INT(value) int(float(value) * canvases_[i].scroll_factor)

namespace Blocks {

ParallaxScrollGroup::ParallaxScrollGroup(ScrollCanvas& first_canvas) 
	: ExpandingInteractive(first_canvas.getExpandingProperties()) {
	canvases_.push_back(SCFactor(first_canvas, 1.0));
}

void ParallaxScrollGroup::addCanvasWithFactor(ScrollCanvas& canvas, const float scroll_factor) {
	canvases_.push_back(SCFactor(canvas, scroll_factor));
}

// SAME FOR ALL
void ParallaxScrollGroup::setScrollPos(const int x, const int y) {
	PS_FOR_ALL_CANVASES_2(setScrollPos, x, y);
}

// Parallax, assumes zero start?
void ParallaxScrollGroup::moveScrollBy(const int dx, const int dy) {
	PS_FOR_ALL_CANVASES_2(setScrollPos, PS_ADJUST_INT(dx), PS_ADJUST_INT(dy));
}

// In px per sec, parallax.
void ParallaxScrollGroup::setScrollSpeed(const float speed_x, const float speed_y) {
	PS_FOR_ALL_CANVASES_2(setScrollPos, PS_ADJUST_FLOAT(speed_x), PS_ADJUST_FLOAT(speed_y));
}

// SAME FOR ALL
void ParallaxScrollGroup::setScrollStart(const int x, const int y) {
	PS_FOR_ALL_CANVASES_2(setScrollStart, x, y);
}

void ParallaxScrollGroup::resetScrollToStart() {
	PS_FOR_ALL_CANVASES(resetScrollToStart);
}

void ParallaxScrollGroup::pauseScroll() {
	PS_FOR_ALL_CANVASES(pauseScroll);
}

void ParallaxScrollGroup::resumeScroll() {
	PS_FOR_ALL_CANVASES(resumeScroll);
}

void ParallaxScrollGroup::event(BlockEvent e) {
	// Send all events to the first canvas
	ScrollCanvas* sc = canvases_[0].canvas;
	if (sc != nullptr) {
		sc->event(e);
	}

	// TODO: Any other events?
}
void ParallaxScrollGroup::action(BlockAction a) {
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
			log::Error("ParallaxScrollGroup", "Unhandled scroll event type!");
			break;
		}
	} else {
		// Otherwise send the action to the first canvas
		ScrollCanvas* sc = canvases_[0].canvas;
		if (sc != nullptr) {
			sc->action(a);
		}
	}
}

void ParallaxScrollGroup::draw() {
	// Draw in reverse, as the first canvas is on top.
	PS_FOR_ALL_CANVASES_REVERSE(draw);
}

void ParallaxScrollGroup::onResize() {
	PS_FOR_ALL_CANVASES_2(setDrawableArea, width_, height_);
}

} // namespace Blocks
