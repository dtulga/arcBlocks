#pragma once

#include "../arc/graphics.h"
#include "../arc/input.h"

#include "event.h"

template<typename T>
inline T min_max(T min, T val, T max) {
	if (val < min) return min;
	if (max != 0 && val > max) return max;
	return val;
}

using namespace arc;

namespace Blocks {

// For all blocks that can be drawn on the screen (use a blank draw function for invisible hitboxes, etc.)
class Drawable {
public:
	Drawable() {}
	Drawable(const int width, const int height) : width_(width), height_(height) {}
	virtual ~Drawable() {}

	// The screen context is guaranteed to be set correctly beforehand, and all draw commands use local coordinates.
	virtual void draw() = 0;

	int width() const { return width_; }
	int height() const { return height_; }

	bool is_visible() { return visible_; } // Canvas won't draw any hidden drawables.
	void show() { visible_ = true; }
	void hide() { visible_ = false; }
	void toggle_visible() { visible_ = !visible_; }

protected:
	int width_ = 0;
	int height_ = 0;
	bool visible_ = true;
};

class ExpandingDrawable : public Drawable {
public:
	struct ExpandingProperties {
		int default_width = 0;
		int min_width = 0;
		int max_width = 0;
		int margin_width = 0; // Only on the expanding side (right)
		int default_height = 0;
		int min_height = 0;
		int max_height = 0;
		int margin_height = 0; // Only on the expanding side (bottom)
		bool expanding_width = true;
		bool expanding_height = true;
	};

	ExpandingDrawable(const ExpandingProperties& ex_props) : Drawable(ex_props.default_width, ex_props.default_height), ex_props_(ex_props) {}

	virtual void setDrawableArea(const int width, const int height) {
		if (ex_props_.expanding_width) {
			width_ = min_max(ex_props_.min_width, width, ex_props_.max_width) - ex_props_.margin_width;
		}
		if (ex_props_.expanding_height) {
			height_ = min_max(ex_props_.min_height, height, ex_props_.max_height) - ex_props_.margin_height;
		}
		onResize();
	}

	// Should only be used for subclasses.
	const ExpandingProperties& getExpandingProperties() const {
		return ex_props_;
	}

protected:
	virtual void onResize() {} // Override this to handle resize events but keep the default sizing code from above.

	ExpandingProperties ex_props_;
};

// Inherit from this class if you want this object to be able to process manager events/actions
class Eventable {
public:
	Eventable() {}
	virtual ~Eventable() {}

	// Override these to enable actions and events for the block.
	virtual void event(BlockEvent e) = 0;
	virtual void action(BlockAction /*a*/) {} // Not all blocks have actions to process.

	size_t connectionID() const { return this_id_; } // == 0 if not registered

	// Use this to enable/disable all event processing (handled by the canvas)
	bool is_enabled() const { return enabled_; }
	void enable() { enabled_ = true; }
	void disable() { enabled_ = false; }
	void toggle_enabled() { enabled_ = !enabled_; }

	// Only called by the manager:
	void registerEventSendQueue(const size_t id, EventQueue& event_queue) { this_id_ = id; event_send_queue_ = &event_queue; }
	void deregisterEventSendQueue() { this_id_ = 0; event_send_queue_ = nullptr; }

	// For events with no other data.
	void sendEvent(const BlockEvent::Type t) const {
		if (event_send_queue_ != nullptr && this_id_ != 0) {
			event_send_queue_->sendEvent(this_id_, BlockEvent(t));
		}
	}

	void sendEvent(const BlockEvent& e) const {
		if (event_send_queue_ != nullptr && this_id_ != 0) {
			event_send_queue_->sendEvent(this_id_, e);
		}
	}

	// Only used for specific blocks like the CollisionSpace, etc.
	void sendCustomOriginEvent(const BlockEvent& e, const size_t id_from) const {
		if (event_send_queue_ != nullptr && this_id_ != 0) {
			event_send_queue_->sendEvent(id_from, e);
		}
	}

private:
	EventQueue* event_send_queue_ = nullptr;
	size_t this_id_ = 0;
	bool enabled_ = true;
};

class ScreenEventable : public Eventable {
public:
	ScreenEventable() : Eventable() {}

	// This is for screen events
	virtual void getEventRegion(int& x, int& y, int& width, int& height) = 0;
};

class Interactive : public Drawable, public ScreenEventable {
public:
	Interactive(const int width, const int height) : Drawable(width, height), ScreenEventable() {}
	
	void showEnable() {
		show();
		enable();
	}

	void hideDisable() {
		hide();
		disable();
	}

	void toggleVisibleEnabled() {
		toggle_visible();
		toggle_enabled();
	}

	void getEventRegion(int& x, int& y, int& width, int& height) override { // Change this to alter the clickable and hitbox region. (All local coordinates.)
		x = 0;
		y = 0;
		width = width_;
		height = height_;
	}
};

class ExpandingInteractive : public ExpandingDrawable, public ScreenEventable {
public:
	ExpandingInteractive(const ExpandingProperties& ex_props) : ExpandingDrawable(ex_props), ScreenEventable() {}

	void showEnable() {
		show();
		enable();
	}

	void hideDisable() {
		hide();
		disable();
	}

	void toggleVisibleEnabled() {
		toggle_visible();
		toggle_enabled();
	}

	void getEventRegion(int& x, int& y, int& width, int& height) override { // Change this to alter the clickable and hitbox region. (All local coordinates.)
		x = 0;
		y = 0;
		width = width_;
		height = height_;
	}
};

// Called every frame
class FrameProcessor {
public:
	FrameProcessor() {}
	virtual ~FrameProcessor() {}

	virtual void frame(double frame_delta_time_msec) = 0; // Time elapsed between this frame and the last.
};

static const constexpr int AlignCentered = -32767;

// Holds a drawable and provides the requested coordinates.
struct BasicAlignment {
	explicit BasicAlignment(Drawable* block) : d(block) {}
	BasicAlignment(Drawable* block, const int x, const int y) : d(block), xpos(x), ypos(y) {}

	Drawable* d = nullptr;
	int xpos = 0;
	int ypos = 0;

	int x() const { return xpos; }
	int y() const { return ypos; }
};

} // namespace Blocks
