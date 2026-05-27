#include <phc/phc.hpp>

#include "audio.hpp"
#include <meta.hpp>
#include <systems/logger.hpp>

namespace audio {
Array<ma_sound *, count> sounds;
ma_engine engine;

Guard init(bool clean) {
  ma_result result;

  result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    LOG_ERROR("Init", "Failed to initialize miniaudio engine");
    return Guard {false};
  }

  LOG_INFO("Init", "Audio engine initialized");

  for (int i = 0; i < count; i++) {
    const String &link = links[i];
    const Path path = get_audio_path() / link;

    ma_sound *sound = new ma_sound();
    ma_result result = ma_sound_init_from_file(&engine, path.c_str(), 0, nullptr, nullptr, sound);

    if (result != MA_SUCCESS) {
      LOG_ERROR("Audio", "Failed to load audio [{}]", link);
      delete sound;
      return (Guard) {false};
    }

    if (clean) {
      ma_sound *old_sound = sounds[i];
      if (old_sound) {
        ma_sound_uninit(old_sound);
        delete old_sound;
      }
      LOG_INFO("Audio", "Deleted Old [{}]", link);
    }

    sounds[i] = sound;
    LOG_INFO("Audio", "Loaded [{}]", link);
  }

  return (Guard) {true};
}

void clean() {
  for (int i = 0; i < sounds.size(); i++) {
    ma_sound *sound = sounds[i];

    if (sound) {
      ma_sound_uninit(sound);
      delete sound;
      LOG_INFO("Audio", "Unloaded [{}]", links[i]);
    }
  }
  ma_engine_uninit(&engine);
  LOG_INFO("Audio", "Uninitialized miniaudio engine");
}
} // namespace audio
