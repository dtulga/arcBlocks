#pragma once

#include <vector>
#include <map>

#include "block.h"
#include "canvas.h"

namespace Blocks {

// Objects do not collide with other objects, and do not move.
// Actors can collide with objects and each other, and can move.

struct CollisionObject {
	CollisionObject() {}
	CollisionObject(
		Canvas::BlockElement* block_element,
		const size_t conn_id,
		const int x_pos, const int y_pos,
		const int width, const int height) 
		: block_handle(block_element), event_id(conn_id),
		  x(x_pos), y(y_pos), w(width), h(height) {}

	// NOT Owned:
	Canvas::BlockElement* block_handle = nullptr;
	size_t event_id = 0; // The event ID for the given block
	
	// Hitbox:
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
};

struct CollisionActor : public CollisionObject {
	// Override this for 'AI' or Player behavior (only used if velocity_mode == CUSTOM)
	virtual void updateVelocity(float frame_delta_time_msec) {} // TODO: Make this into a function pointer instead?

	// In px per sec:
	union {
		float vel_x = 0.0;
		float vel_base; // For FOLLOW_CONSTANT and VEL_FOLLOW_DISTANCE_LINEAR
	};
	union {
		float vel_y = 0.0;
		float vel_distance_factor; // For VEL_FOLLOW_DISTANCE_LINEAR
	};

	// Partial pixels:
	float rem_x = 0.0;
	float rem_y = 0.0;

	// To avoid lagging, add the actor following after the followed actor.
	size_t follow_actor_id = 0;

	typedef uint8_t VEL_MODE;

	static const VEL_MODE VEL_CONSTANT = 0x0; // Uses x and y constant, doesn't use follow_actor_id
	static const VEL_MODE VEL_FOLLOW_SYNC = 0x1; // Matches the other actor's position exactly.
	static const VEL_MODE VEL_FOLLOW_CONSTANT = 0x2; // Angles towards the given actor at the linear velocity.
	static const VEL_MODE VEL_FOLLOW_DISTANCE_LINEAR = 0x3;
	static const VEL_MODE VEL_CUSTOM = 0x10; // Uses updateVelocity, then uses x and y constant

	VEL_MODE velocity_mode = VEL_CONSTANT;
};

struct CollisionChunk {
	// Both are slot vectors, all NOT Owned:
	std::vector<CollisionObject*> objects;
	std::vector<CollisionActor*> actors;
};

class CollisionSpace : public Eventable, public FrameProcessor {
public:
	struct Properties {
		// In chunks:
		size_t grid_x_len = 0; // X length
		size_t grid_y_len = 0; // Y length

		// In px:
		int grid_chunk_size_x = 0;
		int grid_chunk_size_y = 0;
		// WARNING: The chunk size MUST be 2X larger than the largest object/actor/hitbox!

		typedef uint8_t WrapType;

		// For the boundaries of the collision space:
		WrapType wrap_type_x;
		WrapType wrap_type_y;

		static const WrapType WRAP_NONE = 0x1; // Continues off-screen
		static const WrapType WRAP_TOROIDAL = 0x2; // Wraps to top from bottom, etc.
		// TODO:
		static const WrapType WRAP_STOP = 0x3; // Vel = 0 and position = max
		static const WrapType WRAP_ELASTIC = 0x4; // Elastic collision where vel inverted off the wall
	};

	CollisionSpace(const Properties& props, Canvas& canvas);
	~CollisionSpace();

	// Calculates collisions between entities + evaluates speed/velocity to move the blocks as well.
	// Uses a canvas to calculate positions and move blocks.
	// (Scrolling is handled independently, but this can use a ScrollCanvas.)
	// Sends events when collisions are detected.
	// TODO: Also physics-based collisions.

	// Returns the event/connection ID as that is what is used for the actions here as well.
	size_t addObject(const CollisionObject& obj);
	// Uses the drawable dimensions as the hitbox + the event id already assigned.
	size_t addObjectAutoHitbox(Canvas::BlockElement* block_handle);
	size_t addActor(const CollisionActor& actor);

	void removeObjectOrActor(const size_t event_id);

	void actorSetVelocity(const size_t event_id, const float vx, const float vy);
	void actorStop(const size_t event_id) { actorSetVelocity(event_id, 0.0, 0.0); } // Sets velocity to zero

	void event(BlockEvent /*e*/) override {} // TODO: Any events here?
	void action(BlockAction a) override;

	void frame(double frame_delta_time_msec) override;

protected:
	void CheckCollisionsAndSendEvents(
		const int x, const int y, const int x_end, const int y_end,
		const CollisionChunk& c, const size_t event_id);
	void ReassignChunks();
	size_t ChunkFromCoords(int x, int y) const;
	void SendCollisionEvent(const size_t actor, const size_t collided_with) const;
	void UpdateActorPosition(CollisionActor* actor, const float fmsec, const int ox, const int oy, int& x, int& y, bool& moved);

	struct ChunkReassign {
		ChunkReassign(CollisionChunk* chunk_ptr, CollisionActor* actor_ptr) : chunk(chunk_ptr), actor(actor_ptr) {}

		CollisionChunk* chunk;
		CollisionActor* actor;
	};

	Properties props_;

	BlockAction action_move_;

	std::vector<ChunkReassign> reassign_queue_;

	// NOT Owned:
	Canvas* canvas_ = nullptr;

	// Owned:
	CollisionChunk* grid_ = nullptr;

	// This is a map of event_id -> object/actor, and must be a map as objects/actors
	// are evaluated in the order they are added to the event system. (to match with other order)
	// Owned:
	std::map<size_t, CollisionObject*> objects_;
	std::map<size_t, CollisionActor*> actors_;

	size_t grid_len_ = 0; // Total length (in # of grids)

	DELETE_COPY_AND_ASSIGN(CollisionSpace);
};

} // namespace Blocks
