#include <phc/phc.hpp>

#include "texture.hpp"
#include <betr/array.hpp>
#include <utility>

namespace texture {
template <size_t... Is> inline auto make_texture_array(std::index_sequence<Is...>) {
  return betr::Array<betr::String, sizeof...(Is)> {get_texture_path_by_id(std::integral_constant<int, (int)Is> {})...};
}

extern const betr::Array<betr::String, count> links = make_texture_array(std::make_index_sequence<count> {});
} // namespace texture
