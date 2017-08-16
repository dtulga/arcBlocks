#include "level_generator.h"

#include <numeric>

#include "manager.h"

namespace Blocks {

LevelGenerator::~LevelGenerator() {
	for (std::map<size_t, ActiveBlock>::iterator it = active_blocks_.begin(); it != active_blocks_.end(); it++) {
		MultiVisual* mv = it->second.multi_visual;
		if (mv != nullptr) {
			delete mv;
		}
	}
}

size_t LevelGenerator::addLevelElement(LevelElement& element) {
	return addtoslotvectorindex(elements_, &element);
}

void LevelGenerator::removeLevelElement(const size_t id) { // Does not change other elements' id.
	if (id > elements_.size()) return;
	elements_[id] = nullptr;
}

size_t LevelGenerator::addLevelTemplate(LevelTemplate& level) {
	return addtoslotvectorindex(levels_, &level);
}

void LevelGenerator::removeLevelTemplate(const size_t id) { // Does not change other levels' id.
	if (id > levels_.size()) return;
	levels_[id] = nullptr;
}

unsigned int LevelGenerator::seedRandomGenerator(unsigned int seed) {
	if (seed == 0) {
		seed = unsigned int(time(NULL));
	}

	random_engine_.seed(seed);

	return seed;
}

void LevelGenerator::generateLevelFromTemplate(const size_t id) {
	if (id > levels_.size()) return; // TODO: Print error here?

	LevelTemplate* lt = levels_[id];
	if (lt == nullptr) return; // TODO: Print error here?

	const uint32_t g_x_len = lt->grid_x_len;
	const uint32_t g_y_len = lt->grid_y_len;
	const uint32_t grid_len = g_x_len * g_y_len;

	shuffled_groups_.reserve(grid_len); // Max number of groups
	shuffled_cells_.reserve(grid_len); // Max number of cells
	filled_cells_.resize(grid_len); // Filled in cells (-1 is empty)
	std::fill(filled_cells_.begin(), filled_cells_.end(), size_t(-1));

	const size_t e_len = lt->elements.size();
	for (size_t e = 0; e < e_len; e++) {
		LevelElement* element = lt->elements[e];
		if (element == nullptr) continue;

		const uint32_t group_size = max(uint32_t(1), element->generate_grid_region_size);
		const uint32_t group_buf_size = group_size + element->generate_grid_region_buffer;

		// (remainders are discarded)
		const uint32_t group_x_len = g_x_len / group_buf_size;
		const uint32_t group_y_len = g_y_len / group_buf_size;
		const uint32_t group_total_len = group_x_len * group_y_len;

		shuffled_groups_.resize(group_total_len);
		std::iota(shuffled_groups_.begin(), shuffled_groups_.end(), 0); // Fills 0, 1, 2, ... len - 1
		std::shuffle(shuffled_groups_.begin(), shuffled_groups_.end(), random_engine_);

		const uint32_t cell_len = group_size * group_size;

		if (cell_len > 1) {
			shuffled_cells_.resize(cell_len);
			std::iota(shuffled_cells_.begin(), shuffled_cells_.end(), 0); // Fills 0, 1, 2, ... len - 1
		}

		// Max to generate
		const uint32_t gen_n = element->generate_count;

		// Try to generate an element in each chosen group
		for (uint32_t g = 0; g < gen_n && g < group_total_len; g++) {
			const uint32_t try_group = shuffled_groups_[g];
			uint32_t try_cell_start =
				(((try_group / group_x_len) * group_size) * g_x_len) // Y coordinate
				+ ((try_group % group_x_len) * group_size); // X coordinate

			uint32_t try_cell = try_cell_start; // Cell to try
			bool skip_retry = element->replace_mode != LevelElement::RETRY;

			if (cell_len > 1) {
				std::shuffle(shuffled_cells_.begin(), shuffled_cells_.end(), random_engine_);
			}
			for (size_t c = 0; c < cell_len; c++) {
				if (cell_len > 1) {
					// Pick a cell within the group randomly (never pick the buffer, though).
					const uint32_t try_cell_off = shuffled_cells_[c];

					try_cell = try_cell_start
						+ ((try_cell_off / group_size) * g_x_len) // Y coordinate
						+ (try_cell_off % group_size); // X coordinate
				}

				// Attempt to generate the element into this cell				
				if (TryGenerateElementInCell(*element, e, try_cell, g_x_len, g_y_len) || skip_retry) {
					break;
				}
			} // End retry cells
		} // End chose/retry groups, generate n elements
	} // End all elements

	// Add all elements to the ScrollCanvas and CollisionSpace (optional)

	const int g_x_off = lt->grid_x_offset;
	const int g_y_off = lt->grid_y_offset;
	const int g_x_size = lt->grid_x_size;
	const int g_y_size = lt->grid_y_size;

	const size_t empty = -1;

	for (uint32_t y = 0; y < g_y_len; y++) {
		uint32_t y_cell = y * g_x_len;
		for (uint32_t x = 0; x < g_x_len; x++) {
			const size_t e_id = filled_cells_[y_cell + x];
			if (e_id != empty) {
				AddElementAtPos(e_id, g_x_off + (g_x_size * x), g_y_off + (g_y_size * y), g_x_size, g_y_size);
			}
		}
	}
}

bool LevelGenerator::TryGenerateElementInCell(
	LevelElement& element, const size_t element_id,
	const uint32_t cell,
	const uint32_t g_x_len, const uint32_t g_y_len) {

	const bool replace_mode = (element.replace_mode == LevelElement::REPLACE);
	const uint32_t cr_size = element.clear_region_size;
	const size_t empty = -1;

	if (!replace_mode && filled_cells_[cell] != empty) {
		// Skip and Retry will return false if filled, and the generate level loop above will handle retries.
		return false;
	}

	if (cr_size > 0) {
		const uint32_t g_x = cell % g_x_len;
		const uint32_t min_x = g_x > cr_size ? g_x - cr_size : 0;
		const uint32_t max_x = g_x < (g_x_len - 1 - cr_size) ? (g_x + cr_size) : g_x_len - 1;

		const uint32_t g_y = cell / g_x_len;
		const uint32_t min_y = g_y > cr_size ? g_y - cr_size : 0;
		const uint32_t max_y = g_y < (g_y_len - 1 - cr_size) ? (g_y + cr_size) : g_y_len - 1;

		for (uint32_t y = min_y; y <= max_y; y++) {
			const uint32_t y_off = y * g_x_len;
			for (uint32_t x = min_x; x <= max_x; x++) {
				const uint32_t clear_cell = y_off + x;
				if (replace_mode) {
					// Replace always clears here
					filled_cells_[clear_cell] = empty;
				}
				else if (filled_cells_[clear_cell] != empty) {
					// Skip and Retry will return false if filled, and the generate level loop above will handle retries.
					return false;
				}
			}
		}
	}

	// This means all cells were clear or cleared (replace_mode)
	filled_cells_[cell] = element_id;

	return true;
}

void LevelGenerator::AddElementAtPos(const size_t element_id, int x, int y, const int region_x, const int region_y) {
	if (element_id >= elements_.size()) return; // TODO: Print error here?
	LevelElement* element = elements_[element_id];
	if (element == nullptr) return; // TODO: Print error here?

	MultiVisual* mv = new MultiVisual(element->visual_props);

	const size_t event_id = manager.RegisterEventable(*mv);

	if (active_blocks_.count(event_id) != 0) {
		log::Error("LevelGenerator", "Duplicate event id returned from register eventable? Was this block already deleted?");
	}

	const int x_off = element->cell_x_offset;
	const int y_off = element->cell_y_offset;

	x += (x_off == AlignCentered) ? (region_x / 2) - (mv->width() / 2) : x_off;
	y += (y_off == AlignCentered) ? (region_y / 2) - (mv->height() / 2) : y_off;

	Canvas::BlockElement* be = canvas_->addScrollInteractiveAt(*mv, x, y);

	if (collision_ != nullptr) {
		if (element->actor_template != nullptr) {
			CollisionActor actor(*(element->actor_template));

			actor.block_handle = be;
			actor.event_id = event_id;
			actor.x += x;
			actor.y += y;
			// w and h should be in the template
			// + all other actor ids

			collision_->addActor(actor);

		} else if (element->object_template != nullptr) {
			CollisionObject obj(*(element->object_template));

			obj.block_handle = be;
			obj.event_id = event_id;
			obj.x += x;
			obj.y += y;
			// w and h should be in the template

			collision_->addObject(obj);
		}
	}

	active_blocks_[event_id] = ActiveBlock(mv, be);
}

void LevelGenerator::clearLevel() {
	for (std::map<size_t, ActiveBlock>::iterator it = active_blocks_.begin(); it != active_blocks_.end(); it++) {
		const size_t event_id = it->first;
		ActiveBlock& ab = it->second;

		canvas_->removeScrollBlock(ab.block_handle);

		if (collision_ != nullptr) {
			// TODO: Use collision flag in the ActiveBlock?
			collision_->removeObjectOrActor(event_id);
		}

		if (ab.multi_visual != nullptr) {
			delete ab.multi_visual;
		}
	}

	active_blocks_.clear();
}

void LevelGenerator::addBlockAt(const size_t element_id, const int x, const int y) {
	AddElementAtPos(element_id, x, y, 0, 0);
}

void LevelGenerator::addBlockInRegion(
	const size_t element_id, const int x, const int y, const int region_w, const int region_h) {

	AddElementAtPos(element_id, x, y, region_w, region_h);
}

// block_id == conn_id or event_id
void LevelGenerator::removeBlock(const size_t event_id) {
	if (!active_blocks_.count(event_id)) return; // TODO: Print error here?
	ActiveBlock& ab = active_blocks_.at(event_id);

	canvas_->removeScrollBlock(ab.block_handle);

	if (collision_ != nullptr) {
		// TODO: Use collision flag in the ActiveBlock?
		collision_->removeObjectOrActor(event_id);
	}

	if (ab.multi_visual != nullptr) {
		delete ab.multi_visual;
	}

	active_blocks_.erase(event_id);
}

void LevelGenerator::replaceBlock(const size_t event_id, const size_t element_id) {
	if (!active_blocks_.count(event_id)) return; // TODO: Print error here?
	ActiveBlock& ab = active_blocks_.at(event_id);

	if (ab.block_handle == nullptr) return; // TODO: Print error here?

	const int x = ab.block_handle->x;
	const int y = ab.block_handle->y;
	int w = 0;
	int h = 0;

	Drawable* d = ab.block_handle->d;
	if (d != nullptr) {
		// So centering around the center of the old block works.
		int w = d->width();
		int h = d->height();
	}

	removeBlock(event_id);
	AddElementAtPos(element_id, x, y, w, h);
}

void LevelGenerator::action(BlockAction a) {
	if (!a.is_generate()) return;

	switch (a.type) {
	case BlockAction::GenerateLevelTemplate:
		generateLevelFromTemplate(a.id);
		break;
	case BlockAction::GenerateClearLevel:
		clearLevel();
		break;
	case BlockAction::GenerateRemoveBlock:
		removeBlock(a.id);
		break;
	case BlockAction::GenerateRemoveSender:
		removeBlock(a.sender.id);
		break;
	case BlockAction::GenerateAddBlockAt:
		// TODO: Allow a movement/click event to trigger this?
		addBlockAt(a.id, a.position.x, a.position.y);
		break;
	case BlockAction::GenerateReplaceSender:
		replaceBlock(a.sender.id, a.id);
		break;
	default:
		log::Error("LevelGenerator", "Unhandled generate event type!");
		break;
	}
}

} // namespace Blocks
