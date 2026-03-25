#include <phc/phc.hpp>

#include "clay.hpp"
#include "rect.hpp"

#include <betr/namespace.hpp>

#include <betr/vector.hpp>
#include <systems/logger.hpp>

static char *clay_data;
static Clay_Arena arena;

namespace clay {
Guard init(const uvec2 &size) {
  const int clay_data_size = Clay_MinMemorySize();
  clay_data = new char[clay_data_size];
  arena = Clay_CreateArenaWithCapacityAndMemory(clay_data_size, clay_data);

  Clay_Initialize(
      arena, {(float)size.x, (float)size.y},
      {[](Clay_ErrorData error) {
         switch (error.errorType) {
         case CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED: Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2); break;
         case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED:
           Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
           break;
         default: LOG_ERROR("UI/Clay", "{}", error.errorText.chars); return;
         }

         delete[] clay_data;
         const int clay_data_size = Clay_MinMemorySize();
         clay_data = new char[clay_data_size];
         arena = Clay_CreateArenaWithCapacityAndMemory(clay_data_size, clay_data);
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
  return Guard {};
}

void clean() {
  delete[] clay_data;
  LOG_INFO("UI", "Completed Clay Cleanup");
}

void render(const Clay_RenderCommandArray &cmds) {
  Vector<RectComp> rects;

  for (int i = 0; i < cmds.length; i++) {
    const Clay_RenderCommand &cmd = cmds.internalArray[i];
    Clay_BoundingBox bounding_box = {
        roundf(cmd.boundingBox.x), roundf(cmd.boundingBox.y), roundf(cmd.boundingBox.width), roundf(cmd.boundingBox.height)
    };

    switch (cmd.commandType) {
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: rects.emplace_back(bounding_box.x, bounding_box.y, bounding_box.width, bounding_box.height); break;

    default: break;
    }
  }

  renderer::rect::render(rects);
}

} // namespace clay
