#include "shader_list.hpp"
#include <betr/array.hpp>
#include <utility>

namespace shader {
template <size_t... Is> inline auto make_shader_array(std::index_sequence<Is...>) {
  return betr::Array<betr::String, sizeof...(Is)> {get_shader_path_by_id(std::integral_constant<int, (int)Is> {})...};
}

extern const auto links = make_shader_array(std::make_index_sequence<count> {});
} // namespace shader
