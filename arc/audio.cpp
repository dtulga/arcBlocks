#include "audio.h"

namespace arc {

AudioModule audio;

AudioModule::~AudioModule() {
	// This also frees all the Mix_Chunks and Mix_Musics
	deleteallslotvector(sounds_);
	deleteallslotvector(music_);

	if (initialized_) {
		Mix_Quit(); // Close MP3 support.
		Mix_CloseAudio();
	}
}

bool AudioModule::Init() {
	if (initialized_) {
		return true;
	}

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		log::Fatal("AudioModule", string("SDL audio initalization error: ") + SDL_GetError());
		return false;
	}

	// TODO: Params here!
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 735) < 0) { // Mono, 60 chunks per second
		log::Fatal("AudioModule", string("SDL_mixer initalization error: ") + Mix_GetError());
		return false;
	}

	if (Mix_Init(MIX_INIT_MP3) < 0) {
		log::Fatal("AudioModule", string("SDL_mixer mp3 support init error: ") + Mix_GetError());
		return false;
	} else {
		initialized_ = true;
	}

	Mix_AllocateChannels(128); // TODO: Params!

	return initialized_;
}

Sound& AudioModule::SoundFromFile(const string& file_path) {
	string sound_fn = file_path;
	Mix_Chunk* chunk = Mix_LoadWAV(sound_fn.c_str());

	if (chunk == nullptr) {
		throw audio_error("Unable to load sound file: " + file_path + Mix_GetError());
	}

	Sound* s = new Sound(chunk);

	return addtoslotvector(sounds_, s);
}

Music& AudioModule::MusicFromFile(const string& file_path) {
	string music_fn = file_path;
	Mix_Music* music = Mix_LoadMUS(music_fn.c_str());

	if (music == nullptr) {
		throw audio_error("Unable to load music file: " + file_path + Mix_GetError());
	}

	Music* m = new Music(music);

	return addtoslotvector(music_, m);
}

// WARNING: May inadvertently stop another sound, if this one has already finished!
// TODO: Fix this ^
void AudioModule::StopSound(Sound& sound) {
	const int c = sound.channel;
	if (c != -1) {
		Mix_HaltChannel(c);
		sound.channel = -1;
	}
}

void AudioModule::StopAllSounds() {
	Mix_HaltChannel(-1);

	const size_t len = sounds_.size();
	for (size_t i = 0; i < len; i++) {
		Sound* s = sounds_[i];
		if (s != nullptr) {
			s->channel = -1;
		}
	}
}

} // namespace arc
