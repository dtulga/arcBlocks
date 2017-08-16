#include "scene.h"

namespace Blocks {

void Scene::onResize() {
	scenes_[cur_scene_]->setDrawableArea(width_, height_);
}

void Scene::setScene(const size_t id) {
	if (id < scenes_.size()) {
		cur_scene_ = id;
		// In case resizes occured between scenes.
		onResize();
	} else {
		log::Error("Scene", "Tried to set the scene to an invalid id: " + string::itoa(id));
	}
}

void Scene::event(BlockEvent e) {
	scenes_[cur_scene_]->event(e);
}

void Scene::action(BlockAction a) {
	if (a.is_step()) {
		switch (a.type) {
		case BlockAction::SetStep:
			setScene(a.id);
			break;
		case BlockAction::NextStep:
			setScene(cur_scene_ + 1);
			break;
		case BlockAction::PrevStep:
			if (cur_scene_ > 0) {
				setScene(cur_scene_ - 1);
			} else {
				log::Error("Scene", "Tried to go to a previous scene when already at the first scene!");
			}
			break;
		case BlockAction::FirstStep:
			setScene(0);
			break;
		case BlockAction::LastStep:
			setScene(scenes_.size() - 1);
			break;
		default:
			log::Error("Scene", "Unhandled step/scene event type!");
			break;
		}
	} else {
		scenes_[cur_scene_]->action(a);
	}
}

void Scene::draw() {
	// Draw the current scene.
	scenes_[cur_scene_]->draw();
}

} // namespace Blocks
