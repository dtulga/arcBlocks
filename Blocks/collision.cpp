#include "collision.h"

namespace Blocks {

inline bool collision_detect(const int x, const int y, const int x_end, const int y_end, const CollisionObject* c) {
	const int cx = c->x;
	const int cy = c->y;
	const int cxw = cx + c->w;
	const int cyh = cy + c->h;
	return (
		x < cx + cxw &&
		x_end > cx &&
		y < cy + cyh &&
		y_end > cy
		);
}

void CollisionSpace::SendCollisionEvent(const size_t actor, const size_t collided_with) const {
	BlockEvent e(BlockEvent::CollidedWith);
	e.id = collided_with;

	sendCustomOriginEvent(e, actor);
}

void CollisionSpace::CheckCollisionsAndSendEvents(const int x, const int y,
	const int x_end, const int y_end,
	const CollisionChunk& c, const size_t event_id) {

	size_t len = c.objects.size();
	for (size_t i = 0; i < len; i++) {
		if (collision_detect(x, y, x_end, y_end, c.objects[i])) {
			CollisionObject* obj = c.objects[i];
			if (obj != nullptr) {
				SendCollisionEvent(event_id, obj->event_id);
			}
		}
	}

	len = c.actors.size();
	for (size_t i = 0; i < len; i++) {
		if (collision_detect(x, y, x_end, y_end, c.actors[i])) {
			CollisionActor* actor = c.actors[i];
			if (actor != nullptr && actor->event_id != event_id) { // Self-collision is ignored.
				SendCollisionEvent(event_id, actor->event_id);
			}
		}
	}
}

CollisionSpace::CollisionSpace(const Properties& props, Canvas& canvas)
	: props_(props), action_move_(BlockAction::SetPos), canvas_(&canvas) {
	const size_t g_len = props.grid_x_len * props.grid_y_len;
	grid_len_ = g_len;

	grid_ = new CollisionChunk[g_len];
}

CollisionSpace::~CollisionSpace() {
	for (std::map<size_t, CollisionObject*>::iterator it = objects_.begin(); it != objects_.end(); it++) {
		CollisionObject* obj = it->second;
		if (obj != nullptr) {
			delete obj;
		}
	}

	for (std::map<size_t, CollisionActor*>::iterator it = actors_.begin(); it != actors_.end(); it++) {
		CollisionActor* actor = it->second;
		if (actor != nullptr) {
			delete actor;
		}
	}

	delete[] grid_;
}

size_t CollisionSpace::addObject(const CollisionObject& obj) {
	const size_t id = obj.event_id;
	if (objects_.count(id)) {
		log::Error("CollisionSpace", "Adding a duplicate event_id CollisionObject!");
		return 0;
	}

	CollisionObject* new_obj = new CollisionObject(obj);
	
	CollisionChunk& new_chunk = grid_[ChunkFromCoords(obj.x, obj.y)];
	addtoslotvectorindex(new_chunk.objects, new_obj);

	objects_.insert_or_assign(id, new_obj);
	return id;
}

// Uses the drawable dimensions as the hitbox + the event id already assigned.
size_t CollisionSpace::addObjectAutoHitbox(Canvas::BlockElement* block_handle) {
	if (block_handle == nullptr || block_handle->d == nullptr || block_handle->se == nullptr) {
		log::Error("CollisionSpace", "Attempted to add null block handle as an auto-hitbox collision object!");
		return 0;
	}

	const size_t id = block_handle->se->connectionID();
	if (objects_.count(id)) {
		log::Error("CollisionSpace", "Adding a duplicate event_id CollisionObject (as AutoHitbox block_handle)!");
		return 0;
	}

	CollisionObject* new_obj = new CollisionObject(
		block_handle, id,
		block_handle->x, block_handle->y,
		block_handle->d->width(), block_handle->d->height());

	CollisionChunk& new_chunk = grid_[ChunkFromCoords(block_handle->x, block_handle->y)];
	addtoslotvectorindex(new_chunk.objects, new_obj);

	objects_.insert_or_assign(id, new_obj);
	return id;
}

size_t CollisionSpace::addActor(const CollisionActor& actor) {
	const size_t id = actor.event_id;
	if (actors_.count(id)) {
		log::Error("CollisionSpace", "Adding a duplicate event_id CollisionActor!");
		return 0;
	}
	
	CollisionActor* new_actor = new CollisionActor(actor);

	CollisionChunk& new_chunk = grid_[ChunkFromCoords(actor.x, actor.y)];
	addtoslotvectorindex(new_chunk.actors, new_actor);

	actors_.insert_or_assign(id, new_actor);
	return id;
}

void CollisionSpace::removeObjectOrActor(const size_t event_id) {
	if (event_id == 0) {
		log::Error("CollisionSpace", "Attempted to remove a null event id collision object!");
		return;
	}

	if (objects_.count(event_id)) {
		CollisionObject* obj = objects_.at(event_id);
		if (obj == nullptr) {
			return; // TODO: Print error here?
		}

		CollisionChunk& chunk = grid_[ChunkFromCoords(obj->x, obj->y)];

		const size_t len = chunk.objects.size();
		for (size_t i = 0; i < len; i++) {
			if (chunk.objects[i] == obj) {
				if (i == len - 1) {
					chunk.objects.pop_back(); // NOT Owned
					break;
				} else {
					chunk.objects[i] = nullptr; // NOT Owned
					break;
				}
			}
		}

		delete obj;
		objects_.erase(event_id);

	} else if (actors_.count(event_id)) {
		CollisionActor* actor = actors_.at(event_id);
		if (actor == nullptr) {
			return; // TODO: Print error here?
		}

		CollisionChunk& chunk = grid_[ChunkFromCoords(actor->x, actor->y)];
		const size_t len = chunk.actors.size();

		for (size_t i = 0; i < len; i++) {
			if (chunk.actors[i] == actor) {
				if (i == len - 1) {
					chunk.actors.pop_back(); // NOT Owned
					break;
				} else {
					chunk.actors[i] = nullptr; // NOT Owned
					break;
				}
			}
		}

		// Owned
		delete actor;
		actors_.erase(event_id);
	} else {
		// TODO: Should this be a warning?
		//log::Warn("CollisionSpace", "Could not find object/actor to remove based on given event_id!");
		return;
	}
}

void CollisionSpace::actorSetVelocity(const size_t event_id, const float vx, const float vy) {
	if (!actors_.count(event_id)) {
		return; // TODO: Print error here?
	}

	CollisionActor* actor = actors_.at(event_id);
	if (actor == nullptr) {
		return; // TODO: Print error here?
	}

	actor->vel_x = vx;
	actor->vel_y = vy;
}

void CollisionSpace::action(BlockAction a) {
	if (!a.is_collision()) return;

	const size_t id = a.id;

	switch (a.type) {
	case BlockAction::CollisionRemoveObjectOrActor:
		removeObjectOrActor(id);
		break;
	case BlockAction::CollisionActorSetVel:
		actorSetVelocity(id, a.vel.x, a.vel.y);
		break;
	case BlockAction::CollisionActorStop:
		actorStop(id);
		break;
	default:
		log::Error("CollisionSpace", "Unhandled collision event type!");
		break;
	}
}

#define CHUNK_AT(X, Y) grid_[(grid_x_len * Y) + X]

void CollisionSpace::frame(double frame_delta_time_msec) {
	const float fmsec = (float)frame_delta_time_msec;
	
	const size_t grid_x_len = props_.grid_x_len;
	const size_t grid_y_len = props_.grid_y_len;

	// For quadrant optimizations:
	const int chunk_size_x = props_.grid_chunk_size_x;
	const int chunk_size_y = props_.grid_chunk_size_y;
	const int chunk_size_half_x = chunk_size_x / 2;
	const int chunk_size_half_y = chunk_size_y / 2;

	const bool wrap_t_x = props_.wrap_type_x == Properties::WRAP_TOROIDAL;
	const bool wrap_t_y = props_.wrap_type_y == Properties::WRAP_TOROIDAL;

	const int total_size_x = int(grid_x_len) * chunk_size_x;
	const int total_size_y = int(grid_y_len) * chunk_size_y;

	// Update the position of all actors based on their current velocity.
	for (std::map<size_t, CollisionActor*>::iterator it = actors_.begin(); it != actors_.end(); it++) {
		CollisionActor* actor = it->second;
		if (actor == nullptr) continue;

		bool moved = false;

		const int ox = actor->x;
		int x = ox;
		const int oy = actor->y;
		int y = oy;

		const int grid_x = min(size_t(x / chunk_size_x), grid_x_len);
		const int grid_y = min(size_t(y / chunk_size_y), grid_y_len);

		// The chunk this actor is in:
		CollisionChunk& chunk = CHUNK_AT(grid_x, grid_y);

		UpdateActorPosition(actor, fmsec, ox, oy, x, y, moved);

		if (!moved) continue;

		Canvas::BlockElement* be = actor->block_handle;

		be->x = x;
		if (wrap_t_x) {
			if (x < 0) {
				be->x += total_size_x;
			}
			else if (x > total_size_x) {
				be->x -= total_size_x;
			}
		}

		be->y = y;
		if (wrap_t_y) {
			if (y < 0) {
				be->y += total_size_y;
			}
			else if (y > total_size_y) {
				be->y -= total_size_y;
			}
		}

		const bool first_y = grid_y == 0;
		const bool last_y = grid_y == grid_y_len - 1;
		const int off_y_t = first_y && wrap_t_y ? total_size_y : 0;
		const int off_y_b = last_y && wrap_t_y ? -total_size_y : 0;
		const int chunk_min_y = chunk_size_y * grid_y;
		const int chunk_center_y = chunk_min_y + chunk_size_half_y;

		const size_t prev_grid_y = wrap_t_y && first_y ? grid_y_len - 1 : grid_y - 1;
		const size_t next_grid_y = wrap_t_y && last_y ? 0 : grid_y + 1;

		const bool first_x = grid_x == 0;
		const bool last_x = grid_x == grid_x_len - 1;
		const int off_x_l = first_x && wrap_t_x ? total_size_x : 0;
		const int off_x_r = last_x && wrap_t_x ? -total_size_x : 0;

		const int chunk_min_x = chunk_size_x * grid_x;
		const int chunk_center_x = chunk_min_x + chunk_size_half_x;

		const size_t prev_grid_x = wrap_t_x && first_x ? grid_x_len - 1 : grid_x - 1;
		const size_t next_grid_x = wrap_t_x && last_x ? 0 : grid_x + 1;

		// Collision region (TODO: Possible extra collisions with fast diagonal movement?)
		const int x_start = min(x, ox);
		const int x_end = max(x, ox) + actor->w;
		const int y_start = min(y, oy);
		const int y_end = max(y, oy) + actor->h;

		// Check for any collisions
		const size_t event_id = actor->event_id;

		// Always check this chunk.
		CheckCollisionsAndSendEvents(x_start, y_start, x_end, y_end, chunk, event_id);

		const bool overlap_left = x_start < chunk_center_x && (wrap_t_x || !first_x);
		const bool overlap_right = x_end > chunk_center_x && (wrap_t_x || !last_x);

		const bool overlap_top = y_start < chunk_center_y && (wrap_t_y || !first_y);
		const bool overlap_bottom = y_end > chunk_center_y && (wrap_t_y || !last_y);

		// Left chunks
		if (overlap_left) {

			CheckCollisionsAndSendEvents(
				x_start + off_x_l,
				y_start,
				x_end + off_x_l,
				y_end,
				CHUNK_AT(prev_grid_x, grid_y), event_id);

			if (overlap_top) {
				CheckCollisionsAndSendEvents(
					x_start + off_x_l,
					y_start + off_y_t,
					x_end + off_x_l,
					y_end + off_y_t,
					CHUNK_AT(prev_grid_x, prev_grid_y), event_id);
			}
			if (overlap_bottom) {
				CheckCollisionsAndSendEvents(
					x_start + off_x_l,
					y_start + off_y_b,
					x_end + off_x_l,
					y_end + off_y_b,
					CHUNK_AT(prev_grid_x, next_grid_y), event_id);
			}
		}

		// Right chunks
		if (overlap_right) {
			CheckCollisionsAndSendEvents(
				x_start + off_x_r,
				y_start,
				x_end + off_x_r,
				y_end,
				CHUNK_AT(next_grid_x, grid_y), event_id);

			if (overlap_top) {
				CheckCollisionsAndSendEvents(
					x_start + off_x_r,
					y_start + off_y_t,
					x_end + off_x_r,
					y_end + off_y_t,
					CHUNK_AT(next_grid_x, prev_grid_y), event_id);
			}
			if (overlap_bottom) {
				CheckCollisionsAndSendEvents(
					x_start + off_x_r,
					y_start + off_y_b,
					x_end + off_x_r,
					y_end + off_y_b,
					CHUNK_AT(next_grid_x, next_grid_y), event_id);
			}
		}

		// Top and Bottom chunks
		if (overlap_top) {
			CheckCollisionsAndSendEvents(
				x_start,
				y_start + off_y_t,
				x_end,
				y_end + off_y_t,
				CHUNK_AT(grid_x, prev_grid_y), event_id);
		}
		if (overlap_bottom) {
			CheckCollisionsAndSendEvents(
				x_start,
				y_start + off_y_b,
				x_end,
				y_end + off_y_b,
				CHUNK_AT(grid_x, next_grid_y), event_id);
		}

		// Check if it moved to a new chunk
		if (x < chunk_min_x || y < chunk_min_y ||
			x >= chunk_min_x + chunk_size_x || y >= chunk_min_y + chunk_size_y) {
			reassign_queue_.push_back(ChunkReassign(&chunk, actor));
		}
	}
	
	if (reassign_queue_.size() > 0) {
		ReassignChunks();
	}

	// No need to check for collisions if the actor did not move,
	// and objects do not move or collide with each other, so done.
}

float distancef(const float dx, const float dy) {
	return sqrtf(dx * dx + dy * dy); // Squaring always removes negatives in this context, so result is always positive.
}

void velcomponentsf(const float dx, const float dy, const float vel, float& velx, float& vely) {
	// Calculate components based on angles.
	const float angle = atan2f(dy, dx);
	velx = cosf(angle)*vel;
	vely = sinf(angle)*vel;
}

void CollisionSpace::UpdateActorPosition(
	CollisionActor* actor, const float fmsec, const int ox, const int oy, int& x, int& y, bool& moved) {

	const size_t follow_id = actor->follow_actor_id;
	CollisionActor* follow_actor = nullptr;
	int fax = 0;
	int fay = 0;

	if (actor->velocity_mode == CollisionActor::VEL_FOLLOW_SYNC ||
		actor->velocity_mode == CollisionActor::VEL_FOLLOW_CONSTANT ||
		actor->velocity_mode == CollisionActor::VEL_FOLLOW_DISTANCE_LINEAR) {

		if (!actors_.count(follow_id)) return; // TODO: Print error here?
		follow_actor = actors_.at(follow_id);
		if (follow_actor == nullptr) return; // TODO: Print error here?

		fax = follow_actor->x;
		fay = follow_actor->y;
	}

	if (actor->velocity_mode == CollisionActor::VEL_FOLLOW_SYNC) {
		x = fax;
		y = fay;
		moved = (x != ox || y != oy);
		return; // Since this is a direct position update.
	}

	// Used directly for normal velocity, replaced for the others
	float vx = 0.0;
	float vy = 0.0;
	
	if (actor->velocity_mode == CollisionActor::VEL_FOLLOW_CONSTANT) {
		// Get angle, set velx/y based on components
		// Vector pointing from ox to fax
		const float dx = float(fax - ox);
		const float dy = float(fay - oy);
		const float vel = actor->vel_base;
		if (vel != 0.0) {
			velcomponentsf(dx, dy, vel, vx, vy);
		}

	} else if (actor->velocity_mode == CollisionActor::VEL_FOLLOW_DISTANCE_LINEAR) {
		// Get angle, set velx/y based on components + distance factor
		// Vector pointing from ox to fax
		const float dx = float(fax - ox);
		const float dy = float(fay - oy);
		const float vel = actor->vel_base + (actor->vel_distance_factor * distancef(dx, dy));
		if (vel != 0.0) {
			velcomponentsf(dx, dy, vel, vx, vy);
		}

	} else {
		if (actor->velocity_mode == CollisionActor::VEL_CUSTOM) {
			actor->updateVelocity(fmsec);
		}

		vx = actor->vel_x;
		vy = actor->vel_y;
	}

	// Update position based on the updated velocity:

	if (vx != 0.0) {
		const float rx = (fmsec * vx / 1000.0f) + actor->rem_x;
		int dx = (int)rx;
		if (dx != 0) {
			x += dx;
			actor->x = x;
			actor->rem_x = rx - (float)dx;
			moved = true;
		} else {
			actor->rem_x = rx;
		}
	}

	if (vy != 0.0) {
		const float ry = (fmsec * vy / 1000.0f) + actor->rem_y;
		int dy = (int)ry;
		if (dy != 0) {
			y += dy;
			actor->y = y;
			actor->rem_y = ry - (float)dy;
			moved = true;
		} else {
			actor->rem_y = ry;
		}
	}
}

void CollisionSpace::ReassignChunks() {
	const size_t len = reassign_queue_.size();
	for (size_t i = 0; i < len; i++) {
		const ChunkReassign& cr = reassign_queue_[i];

		CollisionChunk* chunk = cr.chunk;
		if (chunk == nullptr) continue; // TODO: Print error here?

		CollisionActor* actor = cr.actor;
		if (actor == nullptr) continue; // TODO: Print error here?

		const size_t a_len = chunk->actors.size();
		for (size_t a = 0; a < a_len; a++) {
			if (chunk->actors[a] == actor) {
				if (a == a_len - 1) {
					chunk->actors.pop_back();
				} else {
					chunk->actors[a] = nullptr; // Since this is a slot vector, and not owned.
				}
				break;
			}
		}

		CollisionChunk& new_chunk = grid_[ChunkFromCoords(actor->x, actor->y)];

		addtoslotvectorindex(new_chunk.actors, actor);
	}

	reassign_queue_.clear(); // Done
}

size_t CollisionSpace::ChunkFromCoords(int x, int y) const {
	const int chunk_size_x = props_.grid_chunk_size_x;
	const int chunk_size_y = props_.grid_chunk_size_y;
	const size_t g_x_len = props_.grid_x_len;
	const size_t g_y_len = props_.grid_y_len;

	if (x < 0) {
		if (props_.wrap_type_x == Properties::WRAP_TOROIDAL) {
			x += chunk_size_x * g_x_len;
		} else {
			x = 0;
		}
	}
	if (y < 0) {
		if (props_.wrap_type_y == Properties::WRAP_TOROIDAL) {
			y += chunk_size_y * g_y_len;
		} else {
			y = 0;
		}
	}
	const size_t cx = min(size_t(x / chunk_size_x), g_x_len);
	const size_t cy = min(size_t(y / chunk_size_y), g_y_len);
	return (cy * g_x_len) + cx;
}

} // namespace Blocks
