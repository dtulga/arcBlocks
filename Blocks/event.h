#pragma once

#include "../arc/input.h"
#include "../arc/ring_buffer.h"

namespace Blocks {

struct BlockEvent {
	typedef uint32_t Type;

	BlockEvent() {}
	explicit BlockEvent(const Type t) : type(t) {}

	Type type = 0;

	size_t origin_id = 0; // 0 is for the manager/screen events.

	union {
		// Used for MouseDown/TouchDown
		struct Screen {
			int x = 0;
			int y = 0;
			int dx = 0;
			int dy = 0;
		} screen;

		size_t id; // Used as the "CollidedWith" id
	};

	static const Type Null = 0;
	static const Type Any = 0; // Only used in connections!

	bool is_screen() const { return (type & B_1) > 0; }
	static const Type PressDown = B_1 + 0x1; // TouchDown / MouseDown
	static const Type PressUp = B_1 + 0x2; // TouchUp / MouseUp
	static const Type PressDrag = B_1 + 0x3; // TouchDrag and MouseMove+Down
	static const Type Hover = B_1 + 0x4; // MouseMove only
	static const Type HoverEnter = B_1 + 0x5;
	static const Type HoverExit = B_1 + 0x6;

	// Used by MultiVisual (and possibly others?)
	bool is_visual() const { return (type & B_2) > 0; }
	static const Type VisualPressDown = B_2 + 0x1;
	static const Type VisualPressUp = B_2 + 0x2;
	static const Type VisualDrag = B_2 + 0x3;
	static const Type VisualDragOut = B_2 + 0x4;

	bool is_button() const { return (type & B_3) > 0; }
	static const Type ButtonPressed = B_3 + 0x1; // Actually fires on press up.
	static const Type ButtonDrag = B_3 + 0x2;
	static const Type ButtonDragOut = B_3 + 0x3;
	static const Type ButtonPressDown = B_3 + 0x4; // Use for selections, not normal button presses, etc.

	bool is_canvas() const { return (type & B_4) > 0; }
	static const Type CanvasPressDown = B_4 + 0x1;
	static const Type CanvasPressUp = B_4 + 0x2;
	static const Type CanvasDrag = B_4 + 0x3;
	static const Type CanvasDragOut = B_4 + 0x4;
	static const Type CanvasResize = B_4 + 0x5; // x and y new size, dx/dy change in size

	bool is_collision() const { return (type & B_5) > 0; }
	static const Type CollidedWith = B_4 + 0x1; // Uses id
	
	// TODO: Other Events //
};

class EventQueue {
public:
	EventQueue(const uint32_t size) : event_queue_(size) {}

	bool sendEvent(const size_t id, BlockEvent event) {
		event.origin_id = id;
		if (!event_queue_.trySend(event)) {
			arc::log::Error("EventQueue", "Event queue full!");
			return false;
		}
		return true;
	}

	bool recvEvent(BlockEvent* event) {
		return event_queue_.tryRecv(event);
	}

protected:
	DELETE_COPY_AND_ASSIGN(EventQueue);

	arc::ring_buffer_local<BlockEvent> event_queue_;
};

struct BlockAction {
	typedef uint32_t Type;

	BlockAction() {}
	explicit BlockAction(const Type t) : type(t) {}

	Type type = -1;

	static const Type Invalid = -1;

	// Handled by the manager (send to destination = 0)
	bool is_system() const { return (type & B_1) > 0; }
	static const Type Quit = B_1 + 0x1;
	static const Type RunFunc = B_1 + 0x2; // Runs func below.

	// These are special actions handled directly by the manager for the given block: (send to destination = block id)
	bool is_block() const { return (type & B_2) > 0; }
	static const Type BlockShow = B_2 + 0x1;
	static const Type BlockHide = B_2 + 0x2;
	static const Type BlockToggleVisible = B_2 + 0x3;
	static const Type BlockEnable = B_2 + 0x4;
	static const Type BlockDisable = B_2 + 0x5;
	static const Type BlockToggleEnabled = B_2 + 0x6;
	// Combo types:
	static const Type BlockShowEnable = B_2 + 0x7;
	static const Type BlockHideDisable = B_2 + 0x8;
	static const Type BlockToggleVisibleEnabled = B_2 + 0x9;

	// Handled by the button, also includes a potential graphical change, and keeps events enabled.
	bool is_button() const { return (type & B_3) > 0; }
	static const Type ButtonEnable = B_3 + 0x1;
	static const Type ButtonDisable = B_3 + 0x2;
	static const Type ButtonToggleEnabled = B_3 + 0x3;

	// Handled by the canvas - send to destination = canvas id and set the data to the block's BlockElement
	bool is_movement() const { return (type & B_4) > 0; }
	static const Type SetPos = B_4 + 0x1; // Uses position
	static const Type MoveBy = B_4 + 0x2; // Uses delta

	// Used by Scene and MultiVisual
	bool is_step() const { return (type & B_5) > 0; }
	static const Type SetStep = B_5 + 0x1; // Uses id
	static const Type NextStep = B_5 + 0x2;
	static const Type PrevStep = B_5 + 0x3;
	static const Type FirstStep = B_5 + 0x4;
	static const Type LastStep = B_5 + 0x5;

	bool is_scroll() const { return (type & B_6) > 0; }
	static const Type SetScrollPos = B_6 + 0x1; // Uses position
	static const Type MoveScrollBy = B_6 + 0x2; // Uses delta
	static const Type ResetScroll = B_6 + 0x3;
	static const Type PauseScroll = B_6 + 0x4;
	static const Type ResumeScroll = B_6 + 0x5;

	// TODO: Change this (and the Variable block, too, to use arc::var when done.)
	bool is_var_int() const { return (type & B_7) > 0; }
	static const Type IntSetValue = B_7 + 0x1;
	static const Type IntInc = B_7 + 0x2;
	static const Type IntDec = B_7 + 0x3;
	static const Type IntAdd = B_7 + 0x4;
	static const Type IntSub = B_7 + 0x5;
	static const Type IntMult = B_7 + 0x6;
	static const Type IntDiv = B_7 + 0x7;

	// Uses id (for some)
	bool is_audio() const { return (type & B_8) > 0; }
	static const Type AudioPlaySound = B_8 + 0x1;
	static const Type AudioStopSound = B_8 + 0x2;
	static const Type AudioStopAllSounds = B_8 + 0x3;
	static const Type AudioPlayMusic = B_8 + 0x4;
	static const Type AudioPlayMusicOnce = B_8 + 0x5;
	static const Type AudioPauseMusic = B_8 + 0x6;
	static const Type AudioResumeMusic = B_8 + 0x7;
	static const Type AudioStopMusic = B_8 + 0x8;
	static const Type AudioRewindMusic = B_8 + 0x9;

	bool is_collision() const { return (type & B_9) > 0; }
	static const Type CollisionRemoveObjectOrActor = B_9 + 0x1; // Uses id
	static const Type CollisionActorSetVel = B_9 + 0x2; // Uses vel
	static const Type CollisionActorStop = B_9 + 0x3;

	bool is_generate() const { return (type & B_10) > 0; }
	static const Type GenerateLevelTemplate = B_10 + 0x1; // Uses id
	static const Type GenerateClearLevel = B_10 + 0x2;
	static const Type GenerateRemoveBlock = B_10 + 0x3; // Uses id
	static const Type GenerateRemoveSender = B_10 + 0x4; // Uses sender.id
	static const Type GenerateAddBlockAt = B_10 + 0x5; // Uses id and position
	static const Type GenerateReplaceSender = B_10 + 0x6; // Uses id, sender.id

	// Data
	union {
		struct {
			int x;
			int y;
		} position;

		struct {
			int dx;
			int dy;
		} delta;

		struct {
			float x;
			float y;
		} vel;

		struct { // TODO: Temporary until arc::var is ready.
			int32_t value;
		} var_int;

		/*struct { // TODO: Temporary until arc::var is ready.
			string* value;
		} var_str;*/

		struct {
			size_t id;
		} sender; // ONLY set for special action types!
	};

	union {
		// Used for step/scene, audio, level, collision, ...
		size_t id;

		// Any other data that is in a custom type, etc.
		// (Currently used by canvas, ...)
		void* data;
	};
};

} // namespace Blocks
