#include <phc/phc.hpp>

#include "audio.hpp"
#include <betr/array.hpp>
#include <utility>

namespace audio {
template <size_t... Is> inline auto make_audio_array(std::index_sequence<Is...>) {
  return betr::Array<betr::String, sizeof...(Is)> {get_audio_path_by_id(std::integral_constant<int, (int)Is> {})...};
}

extern const betr::Array<betr::String, count> links = make_audio_array(std::make_index_sequence<count> {});
} // namespace audio
