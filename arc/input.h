#pragma once

#include <cstdint>
#include <map>
#include <atomic>

#include "graphics.h"

namespace arc {

// TODO: Multiple Screen Support
class InputModule {
public:
	struct Event;
	typedef uint16_t EventType;
	typedef void (*EventHandler)(Event);

	InputModule() { screen_drawing_.store(true); quit_.store(false); quit_immediately_.store(false); }

	void RegisterEventHandler(const EventType event, EventHandler onevent_func) { registered_handlers_[event] = onevent_func; }
	void RegisterDefaultEventHandler(EventHandler onevent_func) { default_handler_ = onevent_func; }
	// This handles interrupt events like app background/foreground - CAN BE CALLED FROM OTHER THREADS!
	void RegisterInterruptEventHandler(EventHandler onevent_func) { interrupt_handler_ = onevent_func; }

	// The double value in the function called is the frame delta time in msec
	void RegisterFrameDrawHandler(Screen& screen, void (*frame_draw_func)(double)) {
		screen_ = &screen; frame_draw_func_ = frame_draw_func;
	}

	// This will not wait when vsync is on (handled by the renderer/OpenGL) or will wait manually when off.
	void SetVsyncAsEnabled(bool enabled = true) { vsync_on_ = enabled; }
	void SetMaximumFrameRate(const unsigned int max_framerate) {
		max_framerate_ = max_framerate; ticks_per_frame_ = SDL_GetPerformanceFrequency() / uint64_t(max_framerate);
	}
	
	// All Quit-related functions are Thread Safe
	// Exit the event loop.
	void Quit() { quit_.store(true); }
    // Cancels a quit event, call this from an event handler in case of unsaved data, etc. for example.
	// (Only works if quit_immediately_ is false!)
	void CancelQuit() { quit_.store(false); }

	void SetQuitImmediately(const bool quit_immediately) { quit_immediately_.store(quit_immediately); }
	// Exit the event loop immediately!
	void InterruptQuit() { quit_.store(true); quit_immediately_.store(true); }

	// Should only be called by the internal handler from SDL! (Thread Safe)
	int HandleInterruptEvent(const SDL_Event* event);

	int ExecEventHandlerLoop(); // Doesn't return until quit!

	// Pass this to catch all events without a specific handler.
	static const EventType DefaultHandler = 0;

	static const EventType EventNull = 0;

	static const EventType QuitEvent = 0x1;

	static const EventType ScreenShow = 0x11;
	static const EventType ScreenHide = 0x12;
	static const EventType ScreenMove = 0x13;
	static const EventType ScreenResize = 0x14;
	static const EventType ScreenFocus = 0x15;
	static const EventType ScreenLoseFocus = 0x16;
	static const EventType ScreenClose = 0x17;

	static const EventType KeyDown = 0x21;
	static const EventType KeyUp = 0x22;

	static const EventType MouseMove = 0x31;
	static const EventType MouseDown = 0x32;
	static const EventType MouseUp = 0x33;
	static const EventType MouseWheel = 0x34;
	// Entering/exiting the screen:
	static const EventType MouseEnter = 0x3E;
	static const EventType MouseExit = 0x3F;

	static const EventType TouchMove = 0x41;
	static const EventType TouchDown = 0x42;
	static const EventType TouchUp = 0x43;

	static const EventType MultiTouch = 0x4F;

	static const EventType ControllerDown = 0x51;
	static const EventType ControllerUp = 0x52;
	static const EventType ControllerAxisMove = 0x53;

	static const EventType JoyDown = 0x61;
	static const EventType JoyUp = 0x62;
	static const EventType JoyAxisMove = 0x63;
	static const EventType JoyBallMove = 0x64;
	static const EventType JoyHatMove = 0x65;

	static const EventType TextEdit = 0x71;
	static const EventType TextEntry = 0x72;

	// Interrupt Events:
	static const EventType AppToBackground = 0xA1;
	static const EventType AppToForeground = 0xA2;
	static const EventType AppLowMemory = 0xA3;
	static const EventType AppTerminating = 0xA4;

	//static const EventType Other = 0xFF; // TODO //

	static const EventType Invalid = -1;

	struct Event {
	public:
		EventType type = Invalid;

		union {
			// KeyDown, KeyUp
			struct {
				uint32_t scancode; // Physical location. (Recommended to use this for game movement controls, etc.)
				uint32_t keycode; // Named key. (They may not match in other input mappings like Dvorak, Spanish, etc.)
				uint16_t mod; // Control, Alt, etc.
				bool is_repeat;
			} key;

			// MouseMove, MouseDown, MouseUp, MouseWheel
			struct {
				uint32_t deviceID;
				// Coordinates of move and click
				union {
					int32_t x;
					int32_t wheelHoriz;
				};
				union {
					int32_t y;
					int32_t wheelVert;
				};
				// Relative motion, only valid for motion events
				int32_t dx;
				int32_t dy;
				union {
					uint8_t button; // Only valid for down/up events
					uint32_t button_state; // Only valid for motion events
				};
			} mouse;

			// ControllerDown, ControllerUp, ControllerAxisMove
			struct {
				uint32_t deviceID;
				union {
					// Index of the control element that was changed.
					uint8_t button;
					uint8_t axis;
				};
				int16_t axis_value; // Down and right are positive, also only valid for ControllerAxisMove.
			} controller;

			// JoyDown, JoyUp, JoyAxisMove, JoyBallMove, JoyHatMove
			struct {
				uint32_t deviceID;
				union {
					// Index of the control element that was changed.
					uint8_t button;
					uint8_t axis;
					uint8_t ball;
					uint8_t hat;
				};
				union {
					int16_t axis_value; // Down and right are positive, also only valid for JoyAxisMove
					int16_t ball_xrel;
					uint8_t hat_position;
				};
				int16_t ball_yrel;
			} joy;

			// TextEdit, TextEntry
			struct {
				string* text; // For TextEdit and TextInput events.
				int32_t start;
				int32_t len;
			} text;

			// TouchDown, TouchUp, TouchMove
			struct {
				SDL_TouchID deviceID; // int64_t ?
				SDL_FingerID fingerID; // int64_t ?
				int32_t x;
				int32_t y;
				// Movement
				int32_t dx;
				int32_t dy;
				float pressure; // 0 to 1
			} touch;

			// MultiTouch
			struct {
				SDL_TouchID deviceID; // int64_t ?
				float rotated;
				float pinched;
				// Centered coords:
				int32_t x;
				int32_t y;
				uint16_t n_fingers;
			} multitouch;

			// ScreenMove, ScreenResize
			struct {
				union {
					int32_t x;
					int32_t width;
				};
				union {
					int32_t y;
					int32_t height;
				};
			} screen;
		};
	};

protected:
	void RunHandler(const Event& e); // Runs the given handler if it exists.

	std::map<EventType, EventHandler> registered_handlers_;
	EventHandler default_handler_ = nullptr;
	EventHandler interrupt_handler_ = nullptr;

	// NOT Owned:
	Screen* screen_ = nullptr;
	void (*frame_draw_func_)(double) = nullptr;

	uint64_t ticks_per_frame_ = 0; // Set later
	unsigned int max_framerate_ = 120;
	bool vsync_on_ = true;

	std::atomic_bool screen_drawing_; // TODO: Per-screen support! (When the window is hidden, frame_draw_func_ is not called)
	
	std::atomic_bool quit_immediately_;
	std::atomic_bool quit_;

};

extern InputModule input;

} // namespace arc
