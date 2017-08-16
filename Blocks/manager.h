#pragma once

#include <vector>
#include <limits>

#include "block.h"
#include "canvas.h"
#include "scene.h"

namespace Blocks {

// Maximum number of events in the queue at any time (per frame and block, depending upon processing by other blocks/connections.)
#define MAX_EVENT_QUEUE 256

//
// Sets up and runs the block system, and handles the event loop, event passing, and all drawing.
//
class Manager {
public:
	struct Connection {
		typedef uint8_t Type;

		explicit Connection(const Type t) : type(t) {}

		Type type = -1;

		// Use the block ID for origin and destinaion:
		size_t origin = 0;
		size_t destination = 0;
		// To send/recv from a group, use the group ID returned by RegisterGroup

		BlockEvent::Type event = -1;
		BlockAction::Type action = -1;

		BlockAction default_data = BlockAction(BlockAction::Invalid); // Optional, set the type to use this data.

		void(*action_func)(BlockEvent); // Sends the original event - ONLY called with RunFunc action type and destination == 0

		union {
			// Used for the IF types:
			bool(*test_func)(BlockEvent) = nullptr;
			bool* test_var;
		};

		// TODO: Multiple origins/destinations? Other types of connections, properties, counters, etc. //

		static const Type Invalid = -1;
		static const Type Always = 0x1;
		static const Type And = 0x2; // TODO
		static const Type Or = 0x3; // TODO
		static const Type IfFunc = 0x11; // Tests the test_func
		static const Type IfVar = 0x12; // Tests the test_var

		static const size_t ID_MANAGER = 0x0;
		static const size_t ID_SELF = -1; // Send back to the block this event was sent from
		static const size_t ID_GROUP_OFFSET = std::numeric_limits<size_t>::max() / 2;
	};

	Manager() : event_queue_(MAX_EVENT_QUEUE) {
		blocks_.push_back((BlockConn*)0x1); // As the zero index is reserved for the manager itself.
		groups_.push_back((BlockGroup*)0x1);
		active_.store(false);
	}
	~Manager();

	// TODO: Multiple windows/screens on desktop support.
	void Init(ExpandingInteractive& main_canvas_or_scene, const ScreenProperties& props);

	int Exec(); // Run the event loop, draw frames, and pass events to the component blocks.

	// This handles events directly from the input module.
	static void Event(InputModule::Event e);
	static void InterruptEvent(InputModule::Event e); // Thread Safe

	// This handles the frame_draw from the input module's event loop.
	static void Draw(const double frame_delta_time_msec);

	// Add FrameProcessors here, as they are run before draw() on the main canvas is called, but after event processing.
	void AddFrameProcessor(FrameProcessor& proc);
	void RemoveFrameProcessor(FrameProcessor& proc);
	
	// To allow this block to participate in the event connection system.
	// If the block only draws or handles screen events from the canvas, then this is unneccessary.
	// Returns the index/id of the block added.
	size_t RegisterEventable(Eventable& block);

	// WARNING: Doesn't remove any connections! (But does remove from all groups)
	void DeregisterEventable(const size_t block_id);

	size_t RegisterGroup(); // Returns the next available group ID
	void DeregisterGroup(size_t group_id); // WARNING: Group must be empty!

	size_t RegisterEventableInGroup(Eventable& block, size_t group_id); // Returns the index/id of the block added.
	void AddEventableToGroup(const size_t block_id, size_t group_id);
	void RemoveEventableFromGroup(const size_t block_id, size_t group_id);
	
	size_t AddEventActionConnection(const Connection& conn);
	// void RemoveEventActionConnection(const size_t conn_id);
	// TODO: Delete connections? + Deregisters that also delete connections?

	// Special functions used when on mobile, and there is special processing outside of the normal blocks.
	// WARNING: HANDLERS MUST BE THREAD SAFE!
	typedef void(*StateHandler)();
	void SetAppToBackgroundHandler(StateHandler onbackground_func) { onbackground_ = onbackground_func; }
	void SetAppToForegroundHandler(StateHandler onforeground_func) { onforeground_ = onforeground_func; }

	Screen* MainScreen() { return main_screen_; }
	double GetFrameDeltaTime() { return frame_delta_time_msec_; } // Time since last frame.
	uint32_t GetFrameCount() { return frame_count_; }

protected:
	DELETE_COPY_AND_ASSIGN(Manager);

	// Called directly by the static handlers above.
	void EventInternal(InputModule::Event e);
	void InterruptEventInternal(InputModule::Event e); // Thread Safe

	void DrawInternal(const double frame_delta_time_msec);

	void ProcessAllBlockEvents();

	void SendActionToBlock(const size_t destination_id, Connection* c, const BlockEvent& e);
	void SendActionToBlock(Eventable* block, Connection* c, const BlockEvent& e);

	struct BlockGroup {
		// All NOT Owned:
		// Note that these blocks must also be already registered!
		std::vector<Eventable*> blocks;
		// Connections from this group (events never sent directly, but they can match blocks in the group.)
		std::vector<Connection*> conn_from;
	};

	struct BlockConn {
		explicit BlockConn(Eventable& e) : block(&e) {}
		// All NOT Owned.
		Eventable* block = nullptr;
		std::vector<Connection*> conn_from; // All connections from this block (to enable fast lookup).
		std::vector<size_t> groups; // Groups this block is a part of (for group-based connections).
	};

	// NOT Owned:
	ExpandingInteractive* main_cs_ = nullptr; // Main canvas or scene manager.
	Screen* main_screen_ = nullptr;

	ScreenProperties screen_props_; // TODO: Multiple screens/windows on Desktop support.

	// NOT Owned:
	std::vector<FrameProcessor*> frame_processors_;

	// Owned:
	std::vector<Connection*> connections_;
	std::vector<BlockConn*> blocks_; // id_ is the index; note that 0 is reserved for the manager itself!
	std::vector<BlockGroup*> groups_;

	EventQueue event_queue_; // For blocks to send events to the management system and through connections to other blocks.

	StateHandler onbackground_ = nullptr;
	StateHandler onforeground_ = nullptr;

	// If the app/program is active (i.e. in the foreground)
	std::atomic_bool active_;

	double frame_delta_time_msec_ = 0.0;
	uint32_t frame_count_ = 0;
};

extern Manager manager;

} // namespace Blocks
