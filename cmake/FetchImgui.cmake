# Fetch imgui source code from Github

include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG        dbb5eeaadffb6a3ba6a60de1290312e5802dba5a # v1.91.8
  SOURCE_DIR     imgui
)

FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_sdl2.cpp
  imgui/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(imgui PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}/imgui
  ${CMAKE_CURRENT_BINARY_DIR}/imgui/backends
)

target_link_libraries(imgui PUBLIC SDL2::SDL2)
