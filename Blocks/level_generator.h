#pragma once

#include <vector>
#include <random>
#include <map>

#include "block.h"
#include "multi_visual.h"
#include "scroll_canvas.h"
#include "collision.h"

namespace Blocks {

// TODO: Future features: random or seed position-based generator + spawner

struct LevelElement {
	MultiVisual::Properties visual_props; // So the MultiVisuals are created/destroyed dynamically.

	// If this is a collision-enabled element: (optional)
	CollisionActor* actor_template = nullptr;
	CollisionObject* object_template = nullptr; // TODO: Auto-hitbox available?

	size_t event_group_id;

	// Generation Properties:
	uint32_t generate_count = 0; // max # attempting to generate

	uint32_t generate_grid_region_size = 1; // Grid cells size to place each object randomly within
	// This makes a group of cells with size dimensions,
	// then picks those cells at random, and places objects randomly within.
	uint32_t generate_grid_region_buffer = 0; // Buffer between the region group of cells

	uint32_t clear_region_size = 0; // # of grid cells to replace or retry if other objects encountered (0 = self only)

	// In px:
	int cell_x_offset = 0; // Use AlignCentered for automatic centering (around the grid position)
	int cell_y_offset = 0;

	typedef uint8_t ReplaceMode;

	static const ReplaceMode RETRY = 0x0;
	static const ReplaceMode SKIP = 0x1;
	static const ReplaceMode REPLACE = 0x2;

	ReplaceMode replace_mode = RETRY;
	// WARNING: Only REPLACE guarantees 100% that all elements will be generated!
};

// Start with a major grid dimension, then spacing is in multiples of grid cells.
// Shuffle grid algorithm, based on spacing, then replace/retry for generated grids, etc.

struct LevelTemplate {
	// In pixels:
	int grid_x_size = 0;
	int grid_y_size = 0;
	// Added objects should be a grid cell or smaller to ensure no overlap.

	// In grid cells:
	uint32_t grid_x_len = 0;
	uint32_t grid_y_len = 0;

	// Use these for a margin off the top/left of the ScrollCanvas + CollisionSpace (optional)
	int grid_x_offset = 0;
	int grid_y_offset = 0;

	// NOT Owned:
	std::vector<LevelElement*> elements;
};

struct ActiveBlock {
	ActiveBlock() {}
	ActiveBlock(MultiVisual* mv, Canvas::BlockElement* be) : multi_visual(mv), block_handle(be) {}
	~ActiveBlock() {};

	// Owned: (But deleted by the LevelGenerator, NOT this struct)
	MultiVisual* multi_visual = nullptr;

	// NOT Owned:
	Canvas::BlockElement* block_handle = nullptr;
};

class LevelGenerator : public Eventable {
public:
	LevelGenerator(ScrollCanvas& canvas) : canvas_(&canvas) {}
	LevelGenerator(ScrollCanvas& canvas, CollisionSpace& collision) : canvas_(&canvas), collision_(&collision) {}
	~LevelGenerator();

	size_t addLevelElement(LevelElement& element);
	void removeLevelElement(const size_t id);

	size_t addLevelTemplate(LevelTemplate& level);
	void removeLevelTemplate(const size_t id); // Does not change other levels' id.

	// Returns the seed used, uses the time as the seed if the given seed == 0
	unsigned int seedRandomGenerator(unsigned int seed = 0);

	void generateLevelFromTemplate(const size_t id);
	void clearLevel();

	void addBlockAt(const size_t element_id, const int x, const int y);
	void addBlockInRegion(const size_t element_id, const int x, const int y, const int region_w, const int region_h);
	void removeBlock(const size_t event_id); // block_id == conn_id or event_id
	void replaceBlock(const size_t event_id, const size_t element_id);

	void event(BlockEvent /*e*/) override {} // TODO: Any events here?
	void action(BlockAction a) override;

protected:
	bool TryGenerateElementInCell(
		LevelElement& element, const size_t element_id,
		const uint32_t cell,
		const uint32_t g_x_len, const uint32_t g_y_len);

	// x, y are the grid cell coords, and the region is the cell size (to center in cells)
	void AddElementAtPos(const size_t element_id, int x, int y, const int region_w, const int region_h);

	// NOT Owned:
	std::vector<LevelElement*> elements_;
	std::vector<LevelTemplate*> levels_;
	// Owned:
	std::map<size_t, ActiveBlock> active_blocks_;

	// NOT Owned:
	ScrollCanvas* canvas_ = nullptr;
	CollisionSpace* collision_ = nullptr; // Optional

	std::mt19937 random_engine_;

	// Used to prevent duplicate generation attempts
	std::vector<uint32_t> shuffled_groups_;
	std::vector<uint32_t> shuffled_cells_; // For sub-group cells
	std::vector<size_t> filled_cells_; // For what has already been generated (-1 when empty)

	DELETE_COPY_AND_ASSIGN(LevelGenerator);
};

} // namespace Blocks
