#include "input.h"

namespace arc {

inline int32_t float_to_int32_round(float x) {
	return (x >= 0.0) ? int32_t(x + 0.5) : int32_t(x - 0.5);
}

InputModule input;

// Thread Safe
int SDLCALL InternalHandleInterruptEvent(void*, SDL_Event* event) { // (First pointer is unused userdata)
	return input.HandleInterruptEvent(event);
}

int InputModule::ExecEventHandlerLoop() { // Doesn't return until quit!
	uint64_t last_frame_start_ticks = 0;
	uint64_t frame_start_ticks = SDL_GetPerformanceCounter();
	const uint64_t ticks_per_sec = SDL_GetPerformanceFrequency();
	ticks_per_frame_ = ticks_per_sec / uint64_t(max_framerate_);
	const double ticks_per_msec_double = double(ticks_per_sec) / 1000.0;

	uint64_t frame_count = 0;
	double frame_msec_per_60 = 0.0;

	// Interrupt Handler
	SDL_AddEventWatch(InternalHandleInterruptEvent, nullptr);

	while (!quit_.load(std::memory_order_relaxed)) {
		last_frame_start_ticks = frame_start_ticks;
		frame_start_ticks = SDL_GetPerformanceCounter();
		const double frame_delta_time_msec = double(frame_start_ticks - last_frame_start_ticks) / ticks_per_msec_double;

		// Handle Events
		SDL_Event event;
		float w = 0.0;
		float h = 0.0;
		while (SDL_PollEvent(&event) != 0) {
			Event e;
			e.type = Invalid;
			switch (event.type) {
				case SDL_QUIT:
				quit_.store(true);
				e.type = QuitEvent;
				break;
				case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_EXPOSED:
					case SDL_WINDOWEVENT_MAXIMIZED:
					case SDL_WINDOWEVENT_RESTORED:
					e.type = ScreenShow;
					screen_drawing_ = true;
					break;
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_MINIMIZED:
					e.type = ScreenHide;
					screen_drawing_ = false;
					break;
					case SDL_WINDOWEVENT_MOVED:
					e.type = ScreenMove;
					e.screen.x = event.window.data1;
					e.screen.y = event.window.data2;
					break;
					case SDL_WINDOWEVENT_RESIZED:
					case SDL_WINDOWEVENT_SIZE_CHANGED:
					e.type = ScreenResize;
					e.screen.width = event.window.data1;
					e.screen.height = event.window.data2;
					break;
					case SDL_WINDOWEVENT_ENTER:
					e.type = MouseEnter;
					break;
					case SDL_WINDOWEVENT_LEAVE:
					e.type = MouseExit;
					break;
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					e.type = ScreenFocus;
					break;
					case SDL_WINDOWEVENT_FOCUS_LOST:
					e.type = ScreenLoseFocus;
					break;
					case SDL_WINDOWEVENT_CLOSE:
					e.type = ScreenClose;
					screen_drawing_ = false;
					break;
					default:
					// Ignore any others for now?
					break;
				}
				break;
				case SDL_KEYDOWN:
				e.type = KeyDown;
				case SDL_KEYUP:
				if (e.type == Invalid) { e.type = KeyUp; }
				e.key.is_repeat = event.key.repeat > 1;
				e.key.scancode = event.key.keysym.scancode;
				e.key.keycode = event.key.keysym.sym;
				e.key.mod = event.key.keysym.mod;
				break;
				case SDL_MOUSEMOTION:
				if (event.motion.which == SDL_TOUCH_MOUSEID) { break; }
				e.type = MouseMove;
				e.mouse.deviceID = event.motion.which;
				e.mouse.x = event.motion.x;
				e.mouse.y = event.motion.y;
				e.mouse.dx = event.motion.xrel;
				e.mouse.dy = event.motion.yrel;
				e.mouse.button_state = event.motion.state;
				break;
				case SDL_MOUSEBUTTONDOWN:
				// TODO: Multiple clicks!
				if (event.button.which == SDL_TOUCH_MOUSEID) { break; }
				e.type = MouseDown;
				case SDL_MOUSEBUTTONUP:
				if (event.button.which == SDL_TOUCH_MOUSEID) { break; }
				if (e.type == Invalid) { e.type = MouseUp; }
				e.mouse.deviceID = event.button.which;
				e.mouse.x = event.button.x;
				e.mouse.y = event.button.y;
				e.mouse.button = event.button.button;
				break;
				case SDL_MOUSEWHEEL:
				if (event.wheel.which == SDL_TOUCH_MOUSEID) { break; }
				e.type = MouseWheel;
				e.mouse.deviceID = event.wheel.which;
				if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
					e.mouse.x = event.wheel.x * -1;
					e.mouse.y = event.wheel.y * -1;
				} else {
					e.mouse.x = event.wheel.x;
					e.mouse.y = event.wheel.y;
				}
				break;
				case SDL_JOYAXISMOTION:
				e.type = JoyAxisMove;
				e.joy.deviceID = event.jaxis.which;
				e.joy.axis = event.jaxis.axis;
				e.joy.axis_value = event.jaxis.value;
				break;
				case SDL_JOYBALLMOTION:
				e.type = JoyBallMove;
				e.joy.deviceID = event.jball.which;
				e.joy.ball = event.jball.ball;
				e.joy.ball_xrel = event.jball.xrel;
				e.joy.ball_yrel = event.jball.yrel;
				break;
				case SDL_JOYHATMOTION:
				e.type = JoyHatMove;
				e.joy.deviceID = event.jhat.which;
				e.joy.hat = event.jhat.hat;
				e.joy.hat_position = event.jhat.value;
				break;
				case SDL_JOYBUTTONDOWN:
				e.type = JoyDown;
				case SDL_JOYBUTTONUP:
				if (e.type == Invalid) { e.type = JoyUp; }
				e.joy.deviceID = event.jbutton.which;
				e.joy.button = event.jbutton.button;
				break;
				case SDL_CONTROLLERAXISMOTION:
				e.type = ControllerAxisMove;
				e.controller.deviceID = event.caxis.which;
				e.controller.axis = event.caxis.axis;
				e.controller.axis_value = event.caxis.value;
				break;
				case SDL_CONTROLLERBUTTONDOWN:
				e.type = ControllerDown;
				case SDL_CONTROLLERBUTTONUP:
				if (e.type == Invalid) { e.type = ControllerUp; }
				e.controller.deviceID = event.cbutton.which;
				e.controller.button = event.cbutton.button;
				break;
#ifndef ARC_MAC
// This is because mac generates duplicate touch events for the touch pad + mouse events.
				case SDL_FINGERDOWN:
				e.type = TouchDown;
				case SDL_FINGERUP:
				if (e.type == Invalid) { e.type = TouchUp; }
				case SDL_FINGERMOTION:
				if (e.type == Invalid) { e.type = TouchMove; }
				e.touch.deviceID = event.tfinger.touchId;
				e.touch.fingerID = event.tfinger.fingerId;
				w = float(screen_->width());
				h = float(screen_->height());
				e.touch.x = float_to_int32_round(event.tfinger.x * w);
				e.touch.y = float_to_int32_round(event.tfinger.y * h);
				e.touch.dx = float_to_int32_round(event.tfinger.dx * w);
				e.touch.dy = float_to_int32_round(event.tfinger.dy * h);
				e.touch.pressure = event.tfinger.pressure;
				break;
#endif
				case SDL_MULTIGESTURE:
				e.type = MultiTouch;
				e.multitouch.deviceID = event.mgesture.touchId;
				e.multitouch.rotated = event.mgesture.dTheta;
				e.multitouch.pinched = event.mgesture.dDist;
				e.multitouch.x = float_to_int32_round(event.mgesture.x * float(screen_->width()));
				e.multitouch.y = float_to_int32_round(event.mgesture.y * float(screen_->height()));
				e.multitouch.n_fingers = event.mgesture.numFingers;
				break;
				case SDL_TEXTEDITING:
				e.type = TextEdit;
				e.text.text = new string(event.edit.text); // TODO: Prevent memory leaks!
				e.text.start = event.edit.start;
				e.text.len = event.edit.length;
				break;
				case SDL_TEXTINPUT:
				e.type = TextEntry;
				e.text.text = new string(event.text.text); // TODO: Prevent memory leaks!
				break;
				/*
				case SDL_JOYDEVICEADDED:
				case SDL_JOYDEVICEREMOVED:
				case SDL_CONTROLLERDEVICEADDED:
				case SDL_CONTROLLERDEVICEREMOVED:
				case SDL_CONTROLLERDEVICEREMAPPED:
				// TODO!
				break;
				case SDL_AUDIODEVICEADDED:
				case SDL_AUDIODEVICEREMOVED:
				// TODO!
				break;
				*/
				case SDL_RENDER_TARGETS_RESET:
				// Ignore as all render targets are automatically cached, or redrawn every frame anyways.
				break;
				case SDL_RENDER_DEVICE_RESET:
                // TODO: Also ignore for same reason as above?
				//render.ReloadAllTextures();
				break;
				case SDL_KEYMAPCHANGED:
				// Ignore currently.
				break;
				case SDL_CLIPBOARDUPDATE:
				// TODO!
				log::Warn("Clipboard not yet supported!");
				break;
				case SDL_DROPFILE:
				case SDL_DROPTEXT:
				case SDL_DROPBEGIN:
				case SDL_DROPCOMPLETE:
				log::Warn("Drag and drop data not yet supported!");
				break;
                case SDL_APP_WILLENTERBACKGROUND:
                case SDL_APP_WILLENTERFOREGROUND:
                case SDL_APP_DIDENTERBACKGROUND:
                case SDL_APP_DIDENTERFOREGROUND:
                case SDL_APP_LOWMEMORY:
                case SDL_APP_TERMINATING:
                // These are all handled by the Interrupt Handler.
                break;
                case SDL_DOLLARGESTURE:
                case SDL_DOLLARRECORD:
				case SDL_SYSWMEVENT:
				default:
				log::Warn("Unsupported event was sent to the event processing loop");
				break;
			}
			if (e.type != Invalid) {
				RunHandler(e);
			}
			if (quit_.load(std::memory_order_relaxed) && quit_immediately_.load(std::memory_order_relaxed)) {
				return 0;
			}
		}

		if (quit_.load(std::memory_order_relaxed) && quit_immediately_.load(std::memory_order_relaxed)) {
			return 0;
		}

		bool drawing = screen_drawing_.load(std::memory_order_relaxed);

		// Draw Frame
		if (frame_draw_func_ != nullptr && drawing) {
			frame_draw_func_(frame_delta_time_msec);
		}

		frame_count++;
		frame_msec_per_60 += frame_delta_time_msec;
		if (frame_count % 60 == 0) {
			const unsigned int fps = (unsigned int) ((60000.0 / frame_msec_per_60) + 0.5);
			frame_msec_per_60 = 0.0;
            if (fps < 60) {
            	// TODO: Disable this if debugging not needed.
                log::Info("Input", "Frame " + string::itoa(frame_count) + (drawing ? " drawing" : " not drawing")
                          + string(", fps: ") + string::itoa(fps));
            }
		}

		if (quit_.load(std::memory_order_relaxed) && quit_immediately_.load(std::memory_order_relaxed)) { // As quit() can be called from within the draw function(s) too.
			return 0;
		}

		// Wait if we're controlling maximum framerate manually/vsync not on.
		// (Otherwise this is handled by the renderer/OpenGL which is used in the frame_draw_func_)
		const uint64_t frame_ticks = SDL_GetPerformanceCounter() - frame_start_ticks;
		if (frame_ticks < ticks_per_frame_) {
			SDL_Delay( (uint32_t) (((ticks_per_frame_ - frame_ticks) * 1000) / ticks_per_sec) );
		}
	}
	return 0;
}

// Thread Safe
int InputModule::HandleInterruptEvent(const SDL_Event* event) {
	Event e;
	e.type = Invalid;

	switch (event->type) {
	case SDL_APP_WILLENTERBACKGROUND:
	case SDL_APP_DIDENTERBACKGROUND:
		screen_drawing_.store(false);
		e.type = AppToBackground;
		break;
	case SDL_APP_WILLENTERFOREGROUND:
	case SDL_APP_DIDENTERFOREGROUND:
		screen_drawing_.store(true);
		e.type = AppToForeground;
		break;
	case SDL_APP_LOWMEMORY:
		e.type = AppLowMemory;
		break;
	case SDL_APP_TERMINATING:
		e.type = AppTerminating;
		break;
	default:
		return 0; // Return value ignored by SDL?
	}

	if (e.type != Invalid && interrupt_handler_ != nullptr) {
		interrupt_handler_(e);
	}

	if (e.type == AppTerminating) {
		InterruptQuit();
	}

	return 1;
}

// TODO: Handler groups, i.e. all keyboard, all mouse, etc.?
void InputModule::RunHandler(const Event& e) {
	const EventType et = e.type;
	if (registered_handlers_.count(et)) {
		registered_handlers_[et](e);
	} else if (default_handler_ != nullptr) {
		default_handler_(e);
	}
}

} // namespace arc
