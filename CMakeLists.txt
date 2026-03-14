cmake_minimum_required(VERSION 3.11)
project(YAGL C CXX)

# Make Cmake Use New OpenGL
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_cpm.cmake)
cpmaddpackage("gh:g-truc/glm#1.0.3")
cpmaddpackage("gh:nothings/stb#master")
cpmaddpackage("gh:fmtlib/fmt#12.1.0")
cpmaddpackage("gh:bfgroup/lyra#1.7.0")
cpmaddpackage("gh:richgel999/miniz#3.1.1")
cpmaddpackage("gh:tasinfarhanmc/betr@0.1.0")
cpmaddpackage(NAME glfw3 GIT_REPOSITORY "https://github.com/glfw/glfw.git"
              GIT_TAG "3.4")

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

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIRS "${SRC_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(yagl "${SRC_FILES}")
target_include_directories(yagl PRIVATE ${INCLUDE_DIRS} ${stb_SOURCE_DIR})
target_link_libraries(
  yagl
  PRIVATE OpenGL::GL
          glm
          glfw
          fmt
          lyra
          betr
          miniz)
target_compile_definitions(yagl PRIVATE GLFW_INCLUDE_NONE
                                        YAGL_SRC_PATH="${SRC_DIR}")
