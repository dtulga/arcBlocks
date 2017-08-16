#include "manager.h"

namespace Blocks {

Manager manager;

Manager::~Manager() {
	// So the 0x1 placeholders are not accidentally deleted.
	blocks_[0] = nullptr;
	groups_[0] = nullptr;
	deleteallslotvector(connections_);
	deleteallslotvector(blocks_);
	deleteallslotvector(groups_);
}

// TODO: Multiple windows/screens on desktop.
void Manager::Init(ExpandingInteractive& main_canvas_or_scene, const ScreenProperties& props) {
	if (!graphics.Init()) {
		log::Fatal("Blocks Manager", "Graphics failed to initialize!");
	}

	main_cs_ = &main_canvas_or_scene;
	screen_props_ = props;
	screen_props_.hidden = true;

	input.RegisterDefaultEventHandler(Event);
	input.RegisterInterruptEventHandler(InterruptEvent);

	// Create the screen but hidden for now. (So everything can be loaded.)
	Screen& screen = graphics.CreateCustomScreen(screen_props_);
	main_screen_ = &screen;

	render.SetScreenContext(screen);

	input.RegisterFrameDrawHandler(screen, Draw);
    
    // Set based on interpreted values in case of fullscreen, mobile, etc.
    main_cs_->setDrawableArea(main_screen_->width(), main_screen_->height());

	active_.store(true);
}

// Run the event loop, draw frames, and pass events to the component blocks.
int Manager::Exec() {
    // Set based on interpreted values in case of fullscreen, mobile, etc.
	main_cs_->setDrawableArea(main_screen_->width(), main_screen_->height());

	main_screen_->show();

	return input.ExecEventHandlerLoop();
}

void Manager::Event(InputModule::Event e) {
	manager.EventInternal(e);
}

// Thread Safe
void Manager::InterruptEvent(InputModule::Event e) {
	manager.InterruptEventInternal(e);
}

// Thread Safe
void Manager::InterruptEventInternal(InputModule::Event e) {
	switch (e.type) {
	case InputModule::AppToBackground:
		active_.store(false);
		if (onbackground_ != nullptr) onbackground_();
		break;
	case InputModule::AppToForeground:
		active_.store(true);
		if (onforeground_ != nullptr) onforeground_();
		break;
	// TODO: LowMemory, Termination events
	}
}

// This handles events directly from the input module.
void Manager::EventInternal(InputModule::Event e) {
	BlockEvent be;
	switch (e.type) {
		case InputModule::ScreenResize:
		// If resize, then resize the main_canvas
		main_cs_->setDrawableArea(e.screen.width, e.screen.height);
		break;
		// If mouse/touch, pass down.
		case InputModule::MouseMove:
		be.screen.x = e.mouse.x;
		be.screen.y = e.mouse.y;
		be.screen.dx = e.mouse.dx;
		be.screen.dy = e.mouse.dy;
		if (e.mouse.button_state & SDL_BUTTON_LMASK) {
			be.type = BlockEvent::PressDrag;
			// Drag events start at x/y and travel by dx/dy
			be.screen.x -= e.mouse.dx;
			be.screen.y -= e.mouse.dy;
		} else {
			be.type = BlockEvent::Hover;
		}
		break;
		case InputModule::MouseDown:
		if (e.mouse.button != SDL_BUTTON_LEFT) {
			break;
		}
		be.type = BlockEvent::PressDown;
		be.screen.x = e.mouse.x;
		be.screen.y = e.mouse.y;
		break;
		case InputModule::MouseUp:
		if (e.mouse.button != SDL_BUTTON_LEFT) {
			break;
		}
		be.type = BlockEvent::PressUp;
		be.screen.x = e.mouse.x;
		be.screen.y = e.mouse.y;
		break;
		// TODO: Right-click/menu button (and 3D touch on iOS too?) //
		case InputModule::MouseWheel:
		// TODO! //
		break;
		// TODO: These two may have no coordinates? //
		case InputModule::MouseEnter:
		be.type = BlockEvent::HoverEnter;
		break;
		case InputModule::MouseExit:
		be.type = BlockEvent::HoverExit;
		break;
		case InputModule::TouchMove:
		be.type = BlockEvent::PressDrag;
		be.screen.x = e.touch.x - e.touch.dx;
		be.screen.y = e.touch.y - e.touch.dy;
		be.screen.dx = e.touch.dx;
		be.screen.dy = e.touch.dy;
		break;
		case InputModule::TouchDown:
		be.type = BlockEvent::PressDown;
		be.screen.x = e.touch.x;
		be.screen.y = e.touch.y;
		break;
		case InputModule::TouchUp:
		be.type = BlockEvent::PressUp;
		be.screen.x = e.touch.x;
		be.screen.y = e.touch.y;
		break;
		case InputModule::MultiTouch:
		// TODO! //
		break;
		default:
		// Ignore other events.
		return;
	}

	if (be.is_screen()) {
		// Send to the Main Canvas (or through the Scene)
		main_cs_->event(be);
	}

	// Process Inter-Block Events Here:
	ProcessAllBlockEvents();
}

void Manager::ProcessAllBlockEvents() {
	BlockEvent e;
	const size_t total_group_count = groups_.size();

	while (event_queue_.recvEvent(&e)) { // More events to process
		const size_t o_id = e.origin_id;
		if (o_id == 0) {
			// Manager itself, which is impossible.
			log::Error("Blocks Manager", "Event sent to queue with invalid zero id!"); continue;
		} else if (o_id >= blocks_.size()) {
			// Since events are never sent from a group id.
			log::Error("Blocks Manager", "Invalid inter-block event source id: " + string::itoa(o_id)); continue;
		}

		BlockConn* bc = blocks_[o_id];
		if (bc == nullptr) {
			log::Error("Blocks Manager", "Event sent to queue from removed/deleted block!"); continue;
		}

		std::vector<Connection*>* conns_ = &(bc->conn_from); // All connections from this block ID.
		size_t group_j = -1;
		const size_t group_len = bc->groups.size();

		while (true) {
			for (size_t i = 0; i < conns_->size(); i++) {
				Connection* c = (*conns_)[i];
				if (c == nullptr || (c->event != BlockEvent::Any && c->event != e.type)) {
					continue; // Check next connection, not a match.
				}

				// Possible match, check other connection conditions.
				if (c->type == Connection::Always) {
					// OK!
				} else if (c->type == Connection::IfFunc) {
					if (c->test_func == nullptr || !(c->test_func(e))) {
						continue; // Not active, otherwise OK.
					}
				} else if (c->type == Connection::IfVar) {
					if (c->test_var == nullptr || !(c->test_var)) {
						continue; // Not active, otherwise OK.
					}
				} else {
					log::Error("Blocks Manager", "Unsupported connection type!"); continue;
				}

				// Send Action
				size_t d_id = c->destination;
				if (d_id == 0) {
					// Manager itself
					if (c->action == BlockAction::Quit) {
						input.Quit(); // Doesn't actually quit immediately, in case of a save dialog, etc.
						continue;
					} else if (c->action == BlockAction::RunFunc) {
						if (c->action_func != nullptr) {
							c->action_func(e);
						} else {
							log::Error("Blocks Manager", "Custom action function missing!");
						}
						continue;
					} else {
						log::Error("Blocks Manager",
							"Invalid action passed to manager: " + string::itoa(c->action)); continue;
					}
				} else if (d_id == Connection::ID_SELF) {
					// Send the action to the origin block
					SendActionToBlock(o_id, c, e);
				} else if (d_id >= Connection::ID_GROUP_OFFSET) {
					// Group destination
					d_id -= Connection::ID_GROUP_OFFSET;
					if (d_id >= groups_.size() || d_id == 0) { // Group zero is also reserved.
						log::Error("Blocks Manager",
							"Invalid inter-block action destination group id: " + string::itoa(d_id)); continue;
					}

					BlockGroup* group = groups_[d_id];

					if (group == nullptr) {
						log::Error("Blocks Manager", "Action sent to removed/deleted group!"); return;
					}

					const size_t g_len = group->blocks.size();

					for (size_t g = 0; g < g_len; g++) {
						Eventable* block = group->blocks[g];
						if (block != nullptr) {
							SendActionToBlock(block, c, e);
						}
					}
				} else {
					// Send the action to the (single) block
					SendActionToBlock(d_id, c, e);
				}
			}

			group_j++;

			while (group_j < group_len &&
				   (bc->groups[group_j] == 0 ||
				    bc->groups[group_j] >= total_group_count ||
					groups_[bc->groups[group_j]] == nullptr)) {
				// Skip invalid groups
				group_j++;
			}

			if (group_j < group_len) {
				conns_ = &(groups_[bc->groups[group_j]]->conn_from);
			} else {
				// Done with groups
				break;
			}
		}
	}
}

void Manager::SendActionToBlock(const size_t destination_id, Connection* c, const BlockEvent& e) {
	if (destination_id >= blocks_.size() || destination_id == 0) {
		log::Error("Blocks Manager",
			"Invalid inter-block action destination id: " + string::itoa(destination_id)); return;
	}

	BlockConn* dest_bc = blocks_[destination_id];
	if (dest_bc == nullptr || dest_bc->block == nullptr) {
		log::Error("Blocks Manager", "Action sent to removed/deleted block!"); return;
	}

	SendActionToBlock(dest_bc->block, c, e);
}

void Manager::SendActionToBlock(Eventable* block, Connection* c, const BlockEvent& e) {
	BlockAction a(c->action);

	if (c->default_data.type != BlockAction::Invalid) {
		a = c->default_data;
		a.type = c->action;
	}

	if (a.is_movement()) {
		if (a.type == BlockAction::SetPos) {
			a.position.x = e.screen.x;
			a.position.y = e.screen.y;
		} else if (a.type == BlockAction::MoveBy) {
			a.delta.dx = e.screen.dx;
			a.delta.dy = e.screen.dy;
		}
	} else if (a.is_block()) {
		Drawable* d = nullptr;
		switch (a.type) {
		case BlockAction::BlockEnable:
			block->enable();
			break;
		case BlockAction::BlockDisable:
			block->disable();
			break;
		case BlockAction::BlockToggleEnabled:
			block->toggle_enabled();
			break;
		case BlockAction::BlockShow:
		case BlockAction::BlockHide:
		case BlockAction::BlockToggleVisible:
		case BlockAction::BlockShowEnable:
		case BlockAction::BlockHideDisable:
		case BlockAction::BlockToggleVisibleEnabled:
			d = dynamic_cast<Drawable*>(block);
			if (d == nullptr) {
				log::Fatal("Blocks Manager", "Attempted to show/hide a non-drawable block!");
			}
			switch (a.type) {
			case BlockAction::BlockShow:
				d->show();
				break;
			case BlockAction::BlockHide:
				d->hide();
				break;
			case BlockAction::BlockToggleVisible:
				d->toggle_visible();
				break;
			case BlockAction::BlockShowEnable:
				d->show();
				block->enable();
				break;
			case BlockAction::BlockHideDisable:
				d->hide();
				block->disable();
				break;
			case BlockAction::BlockToggleVisibleEnabled:
				d->toggle_visible();
				block->toggle_enabled();
				break;
			default:
				log::Error("Blocks Manager", "Unknown block-specific action!");
				break;
			}
			break;
		default:
			log::Error("Blocks Manager", "Unknown block-specific action!");
			break;
		}
		return; // These actions are done directly without passing to the block.
	} else if (a.is_generate()) {
		if (a.type == BlockAction::GenerateRemoveSender || a.type == BlockAction::GenerateReplaceSender) {
			a.sender.id = e.origin_id;
		}
	}

	block->action(a);
}

void Manager::Draw(const double frame_delta_time_msec) {
	manager.DrawInternal(frame_delta_time_msec);
}

// This handles the frame_draw from the input module's event loop.
void Manager::DrawInternal(const double frame_delta_time_msec) {
	if (!active_.load(std::memory_order_relaxed)) return;

	frame_delta_time_msec_ = frame_delta_time_msec;
	frame_count_++;

	// Process any physics/etc. before drawing.
	const size_t len = frame_processors_.size();
	for (size_t i = 0; i < len; i++) {
		FrameProcessor* fp = frame_processors_[i];
		if (fp != nullptr) {
			fp->frame(frame_delta_time_msec);
		}
	}

	if (!active_.load(std::memory_order_relaxed)) return;

	render.ClearDrawOffset(); // As the main canvas fills the entire screen.
	render.Clear(white);

	main_cs_->draw();

	if (!active_.load(std::memory_order_relaxed)) return;

	render.RenderFrameDone();
}

void Manager::AddFrameProcessor(FrameProcessor& proc) {
	addtoslotvectorindex(frame_processors_, &proc);
}

void Manager::RemoveFrameProcessor(FrameProcessor& proc) {
	// Don't delete as the frame processor object is not owned here, and may be re-used or re-added later on.
	deletefromslotvector(frame_processors_, &proc, false);
}

// To allow this block to participate in the event connection system.
// If the block only draws or handles screen events from the canvas, then this is unneccessary.
size_t Manager::RegisterEventable(Eventable& block) {
	const size_t id = addtoslotvectorindex(blocks_, new BlockConn(block));

	block.registerEventSendQueue(id, event_queue_); // So this block can send events to the manager and other blocks.

	return id;
}

void Manager::DeregisterEventable(const size_t block_id) {
	if (block_id >= blocks_.size()) {
		// TODO: Print error here?
		return;
	}

	BlockConn* bc = blocks_[block_id];

	if (bc == nullptr) {
		return; // OK if already deleted.
	}

	Eventable* e = blocks_[block_id]->block;

	if (e != nullptr) {
		e->deregisterEventSendQueue();
	}/* else {
		// TODO: Print error here?
	}*/

	for (size_t i = 0; i < bc->groups.size(); i++) {
		RemoveEventableFromGroup(block_id, bc->groups[i]);
	}
	
	delete blocks_[block_id];

	blocks_[block_id] = nullptr;
}

size_t Manager::RegisterGroup() { // Returns the next available group ID
	return addtoslotvectorindex(groups_, new BlockGroup()) + Connection::ID_GROUP_OFFSET;
}

// TODO: Remove the groups each block in this group is a part of from their BlockConn(s)
// OR: Make sure all blocks are removed from this group first, before it can be deleted?
void Manager::DeregisterGroup(size_t group_id) {
	if (group_id >= Connection::ID_GROUP_OFFSET) {
		group_id -= Connection::ID_GROUP_OFFSET;
	}

	if (group_id >= groups_.size() || group_id == 0) {
		// TODO: Print error here?
		return;
	}

	BlockGroup* group = groups_[group_id];

	if (group == nullptr) {
		return; // OK if already deleted.
	}

	delete groups_[group_id];

	groups_[group_id] = nullptr;
}

size_t Manager::RegisterEventableInGroup(Eventable& block, const size_t group_id) { // Returns the index/id of the block added.
	const size_t id = RegisterEventable(block);

	AddEventableToGroup(id, group_id);

	return id;
}

void Manager::AddEventableToGroup(const size_t block_id, size_t group_id) {
	if (group_id >= Connection::ID_GROUP_OFFSET) {
		group_id -= Connection::ID_GROUP_OFFSET;
	}

	if (group_id >= groups_.size() || group_id == 0 || block_id >= blocks_.size()) {
		// TODO: Print error here?
		return;
	}

	BlockGroup* group = groups_[group_id];
	BlockConn* bc = blocks_[block_id];

	if (group == nullptr || bc == nullptr) {
		// TODO: Print error here?
		return;
	}

	addtoslotvectorindex(group->blocks, bc->block);

	addtozerovector(bc->groups, group_id);
}

void Manager::RemoveEventableFromGroup(const size_t block_id, size_t group_id) {
	if (group_id >= Connection::ID_GROUP_OFFSET) {
		group_id -= Connection::ID_GROUP_OFFSET;
	}

	if (group_id >= groups_.size() || group_id == 0 || block_id >= blocks_.size()) {
		// TODO: Print error here?
		return;
	}

	BlockGroup* group = groups_[group_id];
	BlockConn* bc = blocks_[block_id];

	if (group == nullptr || bc == nullptr) {
		// TODO: Print error here?
		return;
	}

	Eventable* e = bc->block;

	if (e == nullptr) {
		// TODO: Print error here?
		return;
	}

	deletefromslotvector(group->blocks, e, false); // Don't actually delete as the Eventable is not owned.

	removefromzerovector(bc->groups, group_id);
}

size_t Manager::AddEventActionConnection(const Connection& conn) {
	const size_t o = conn.origin;
	const size_t d = conn.destination;
	const size_t bc_len = blocks_.size();
	// Destination can be the manager (0), and both can be a group.
	if (o == 0 ||
		(o >= bc_len && !(o >= Connection::ID_GROUP_OFFSET && (o - Connection::ID_GROUP_OFFSET) < groups_.size())) ||
		(d >= bc_len && !(d >= Connection::ID_GROUP_OFFSET && (d - Connection::ID_GROUP_OFFSET) < groups_.size())
			&& d != Connection::ID_SELF)) {
		log::Error("Invalid connection added to manager (invalid origin/destination)!");
		return -1; // TODO: Should connection zero be returned/reserved instead?
	}

	if (conn.type == Connection::IfFunc && conn.test_func == nullptr) {
		log::Error("Blocks Manager", "Invalid IfFunc connection - no test function!"); return -1;
	}
	if (conn.type == Connection::IfVar && conn.test_var == nullptr) {
		log::Error("Blocks Manager", "Invalid IfFunc connection - no test function!"); return -1;
	}

	Connection* c = new Connection(conn);
	const size_t conn_id = addtoslotvectorindex(connections_, c); // Copy data.

	if (o >= Connection::ID_GROUP_OFFSET) { // Is a group-based connection.
		groups_[o - Connection::ID_GROUP_OFFSET]->conn_from.push_back(c);
	} else {
		blocks_[o]->conn_from.push_back(c);
	}

	return conn_id;
}

} // namespace Blocks
