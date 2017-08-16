#pragma once

#include "block.h"

#include "../arc/audio.h"

namespace Blocks {

// Plays sounds from actions or functions
class SoundGenerator : public Eventable {
public:
	SoundGenerator() {}
	~SoundGenerator() {}

	size_t addSound(Sound& sound);
	void removeSound(const size_t id);

	void playSound(const size_t id);
	void stopSound(const size_t id);
	void stopAllSounds();

	size_t addMusic(Music& music);
	void removeMusic(const size_t id);
	
	void playMusic(const size_t id); // Also changes the active music.
	void playMusicOnce(const size_t id); // Also changes the active music.
	void pauseMusic(); //TODO: const size_t id);
	void resumeMusic(); //const size_t id);
	void stopMusic(); //const size_t id);
	void rewindMusic(); //const size_t id); // Start again from the beginning

	void event(BlockEvent /*e*/) override {} // TODO: Any events here?
	void action(BlockAction a) override;

protected:
	// Not owned:
	// id == vector index
	std::vector<Sound*> sounds_;
	std::vector<Music*> music_;
};

//
// Inline functions:
//

inline void SoundGenerator::stopAllSounds() {
	audio.StopAllSounds();
}

inline void SoundGenerator::pauseMusic() {
	audio.PauseMusic();
}

inline void SoundGenerator::resumeMusic() {
	audio.ResumeMusic();
}

inline void SoundGenerator::stopMusic() {
	audio.StopMusic();
}

inline void SoundGenerator::rewindMusic() { // Start again from the beginning
	audio.RewindMusic();
}

} // namespace Blocks
