# Fetch imgui source code from Github

include(FetchContent)

FetchContent_Declare(
  imgui
  URL      https://github.com/ocornut/imgui/archive/refs/tags/v1.92.6.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP true
)

FetchContent_MakeAvailable(imgui)

message("Fetched imgui source code from Github: ${imgui_SOURCE_DIR}")

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC SDL2::SDL2)
