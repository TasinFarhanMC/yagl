#include "clay.h"
#include <math.hpp>

inline vec2 as_vec2(const Clay_Vector2 &v) { return vec2(v.x, v.y); }
inline Clay_Vector2 as_clay2(const vec2 &v) { return Clay_Vector2(v.x, v.y); }

namespace clay {
void clean();

struct Guard {
  ~Guard() { clean(); }
};

Guard init(const uvec2 &size);
} // namespace clay
