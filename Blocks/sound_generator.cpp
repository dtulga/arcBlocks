#include "sound_generator.h"

namespace Blocks {

#define SOUND_CHECK(func) if (id < sounds_.size()) { \
	Sound* s = sounds_[id]; \
	if (s != nullptr) { \
		audio.func(*s); \
	} \
} /* TODO: Print error here? */

#define MUSIC_CHECK(func) if (id < music_.size()) { \
	Music* m = music_[id]; \
	if (m != nullptr) { \
		audio.func(*m); \
	} \
} /* TODO: Print error here? */

size_t SoundGenerator::addSound(Sound& sound) {
	return addtoslotvectorindex(sounds_, &sound);
}

void SoundGenerator::removeSound(const size_t id) {
	if (id < sounds_.size()) {
		sounds_[id] = nullptr;
	}
}

void SoundGenerator::playSound(const size_t id) {
	SOUND_CHECK(PlaySound);
}

void SoundGenerator::stopSound(const size_t id) {
	SOUND_CHECK(StopSound);
}

size_t SoundGenerator::addMusic(Music& music) {
	return addtoslotvectorindex(music_, &music);
}

void SoundGenerator::removeMusic(const size_t id) {
	if (id < music_.size()) {
		music_[id] = nullptr;
	}
}

void SoundGenerator::playMusic(const size_t id) { // Also changes the active music.
	MUSIC_CHECK(PlayMusic);
}

void SoundGenerator::playMusicOnce(const size_t id) { // Also changes the active music.
	MUSIC_CHECK(PlayMusicOnce);
}

void SoundGenerator::action(BlockAction a) {
	if (a.is_audio()) {
		const size_t id = a.id;

		switch (a.type) {
		case BlockAction::AudioPlaySound:
			playSound(id);
			break;
		case BlockAction::AudioStopSound:
			stopSound(id);
			break;
		case BlockAction::AudioStopAllSounds:
			stopAllSounds();
			break;
		case BlockAction::AudioPlayMusic:
			playMusic(id);
			break;
		case BlockAction::AudioPlayMusicOnce:
			playMusicOnce(id);
			break;
		case BlockAction::AudioPauseMusic:
			pauseMusic();
			break;
		case BlockAction::AudioResumeMusic:
			resumeMusic();
			break;
		case BlockAction::AudioStopMusic:
			stopMusic();
			break;
		case BlockAction::AudioRewindMusic:
			rewindMusic();
			break;
		default:
			log::Error("SoundGenerator", "Unknown audio action type!");
			break;
		}
	}
}

} // namespace Blocks
