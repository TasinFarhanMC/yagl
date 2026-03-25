#include "clay.hpp"

#include <betr/namespace.hpp>

#include <betr/vector.hpp>
#include <logger.hpp>

static Vector<char> clay_data;
static Clay_Arena arena;

namespace clay {
void init(const uvec2 &window_dim) {
  clay_data.resize(Clay_MinMemorySize());
  arena = Clay_CreateArenaWithCapacityAndMemory(clay_data.size(), clay_data.data());

  Clay_Initialize(
      arena, {(float)window_dim.x, (float)window_dim.y},
      {[](Clay_ErrorData error) {
         switch (error.errorType) {
         case CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED: Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2); break;
         case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED:
           Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
           break;
         default: LOG_ERROR("UI/Clay", "{}", error.errorText.chars); return;
         }

         clay_data.resize(Clay_MinMemorySize());
         arena = Clay_CreateArenaWithCapacityAndMemory(clay_data.size(), clay_data.data());
       },
       nullptr}
  );

  Clay_SetMeasureTextFunction(
      [](Clay_StringSlice text, Clay_TextElementConfig *config, void *user_data) -> Clay_Dimensions {
        return {0, 0}; // DUMMY
      },
      nullptr
  );

  LOG_INFO("UI", "Clay Initialized");
}

void clean() {
  clay_data.clear();
  LOG_INFO("UI", "Completed Clay Cleanup");
}
} // namespace clay
