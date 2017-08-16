#pragma once

#include <exception>
#include <vector>

#include "graphics.h"

#ifdef __APPLE__
	#ifdef ARC_IOS
		#include "SDL_mixer.h"
	#else
		#include <SDL2_mixer/SDL_mixer.h>
	#endif
#elif __unix__
	#include <SDL2/SDL_mixer.h>
#else
	#include <SDL_mixer.h>
#endif

namespace arc {

/*

TODO:

Playing a sound, then playing another, then stopping the first
will stop the second if the second is on the same channel.

Custom mixer with support for multiple music playing simultaneously, compressor, etc.

*/

// Short sound effect or sample
struct Sound {
	Sound(Mix_Chunk* s) : sample(s) {}
	~Sound() {
		Mix_FreeChunk(sample);
	}

	Mix_Chunk* sample = nullptr;
	int channel = -1; // Last channel played/playing on (for stop).
};

// Background music (only one can play at a time currently in SDL_mixer)
struct Music {
	Music(Mix_Music* m) : music(m) {}
	~Music() {
		Mix_FreeMusic(music);
	}

	Mix_Music* music = nullptr;
};

class AudioModule {
public:
	AudioModule() {};
	~AudioModule();

	// Must be called AFTER graphics.Init()
	bool Init();

	Sound& SoundFromFile(const string& file_path);
	Music& MusicFromFile(const string& file_path);

	// repeat_count = 0 is once, -1 same as Loop
	void PlaySound(Sound& sound, const int repeat_count = 0);
	// Loops indefinitely until stopped.
	void PlaySoundLoop(Sound& sound) { PlaySound(sound, -1); }

	void SetSoundVolume(Sound& sound, const int volume);
	int GetSoundVolume(Sound& sound);

	void SetSoundMasterVolume(const int volume);
	int GetSoundMasterVolume();

	void StopSound(Sound& sound);
	void StopAllSounds();

	// Also used to replace the music immediately, repeat_count = # of times to loop after first.
	void PlayMusic(Music& music, const int repeat_count = -1);
	void PlayMusicOnce(Music& music) { PlayMusic(music, 0); }
	void PauseMusic();
	void ResumeMusic();
	void StopMusic();
	void RewindMusic();

	void SetMusicVolume(const int volume); // From 0 to 128?
	int GetMusicVolume();
	// TODO: Fade transition from music 1 to 2
	// Fade in/out in general
	// Effects/panning/etc.
	// Music or sound done callbacks/events

protected:
	// For auto-memory management.
	std::vector<Sound*> sounds_;
	std::vector<Music*> music_;

	bool initialized_ = false;

private:
	DELETE_COPY_AND_ASSIGN(AudioModule);
};

extern AudioModule audio;

class audio_error : public std::runtime_error {
public:
	explicit audio_error(const std::string& what_arg) : std::runtime_error(what_arg) {}
	explicit audio_error(const char* what_arg) : std::runtime_error(what_arg) {}
	explicit audio_error(const arc::string& what_arg) : std::runtime_error((string(what_arg)).c_str()) {}
};

//
// Inline functions:
//

inline void AudioModule::PlaySound(Sound& sound, const int repeat_count) {
	sound.channel = Mix_PlayChannel(-1, sound.sample, repeat_count);
}

inline void AudioModule::SetSoundVolume(Sound& sound, const int volume) {
	Mix_VolumeChunk(sound.sample, volume);
}

inline int AudioModule::GetSoundVolume(Sound& sound) {
	return Mix_VolumeChunk(sound.sample, -1);
}

inline void AudioModule::SetSoundMasterVolume(const int volume) {
	Mix_Volume(-1, volume);
}

inline int AudioModule::GetSoundMasterVolume() {
	return Mix_Volume(-1, -1);
}

// Also used to replace the music immediately.
inline void AudioModule::PlayMusic(Music& music, const int repeat_count) {
	Mix_PlayMusic(music.music, repeat_count);
}

inline void AudioModule::PauseMusic() {
	Mix_PauseMusic();
}

inline void AudioModule::ResumeMusic() {
	Mix_ResumeMusic();
}

inline void AudioModule::StopMusic() {
	Mix_HaltMusic();
}

inline void AudioModule::RewindMusic() {
	Mix_RewindMusic();
}

// From 0 to 128?
inline void AudioModule::SetMusicVolume(const int volume) {
	Mix_VolumeMusic(volume);
}

inline int AudioModule::GetMusicVolume() {
	return Mix_VolumeMusic(-1);
}

} // namespace arc
