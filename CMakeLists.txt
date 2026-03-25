cmake_minimum_required(VERSION 3.11)
project(YAGL C CXX)

# Make Cmake Use New OpenGL
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_cpm.cmake")
cpmaddpackage("gh:g-truc/glm#1.0.3")
cpmaddpackage("gh:fmtlib/fmt#12.1.0")
cpmaddpackage("gh:bfgroup/lyra#1.7.0")
cpmaddpackage("gh:richgel999/miniz#3.1.1")
cpmaddpackage("gh:tasinfarhanmc/betr@0.1.1")
cpmaddpackage(NAME glfw3 GIT_REPOSITORY "https://github.com/glfw/glfw.git"
              GIT_TAG "3.4")

cpmaddpackage(
  NAME stb_image URL
  "https://cdn.jsdelivr.net/gh/nothings/stb@2b2e64b/stb_image.h"
  DOWNLOAD_NO_EXTRACT TRUE)

add_library(stb_image STATIC "${VENDOR_DIR}/stb_image.c")
target_include_directories(stb_image PUBLIC ${stb_image_SOURCE_DIR})

cpmaddpackage(
  NAME clay URL
  "https://github.com/nicbarker/clay/releases/download/v0.14/clay.h"
  DOWNLOAD_NO_EXTRACT TRUE)

add_library(clay STATIC "${VENDOR_DIR}/clay.c")
target_include_directories(clay PUBLIC ${clay_SOURCE_DIR})

add_library(glad STATIC "${VENDOR_DIR}/gl.c")
target_include_directories(glad PUBLIC "${INCLUDE_DIR}/glad")

# cpmaddpackage( NAME Imgui GIT_REPOSITORY "https://github.com/ocornut/imgui"
# VERSION 1.92.1 DOWNLOAD_ONLY)
#
# add_library( imgui STATIC ${Imgui_SOURCE_DIR}/imgui.cpp
# ${Imgui_SOURCE_DIR}/imgui_demo.cpp ${Imgui_SOURCE_DIR}/imgui_draw.cpp
# ${Imgui_SOURCE_DIR}/imgui_widgets.cpp ${Imgui_SOURCE_DIR}/imgui_tables.cpp
# ${Imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
# ${Imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
#
# target_include_directories(imgui PUBLIC ${Imgui_SOURCE_DIR}
# ${Imgui_SOURCE_DIR}/backends) target_link_libraries(imgui PUBLIC glfw)

file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(yagl ${SRC_FILES})
target_include_directories(yagl PRIVATE ${SRC_DIR})
target_link_libraries(
  yagl
  PRIVATE glfw
          OpenGL::GL
          glad
          glm
          lyra
          fmt
          betr
          miniz
          stb_image
          clay)
target_compile_definitions(yagl PRIVATE GLFW_INCLUDE_NONE
                                        YAGL_SRC_PATH="${SRC_DIR}")
