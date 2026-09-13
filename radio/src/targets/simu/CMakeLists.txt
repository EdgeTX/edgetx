
option(SIMU_TARGET "Configure simu target" ON)
if(NOT SIMU_TARGET)
  message(STATUS "simu target disabled")
  return()
endif()

# Widget Studio dev hooks (simu target only, never release firmware):
#   - simu Lua module (api_simu.cpp), gated by WIDGET_STUDIO
#   - deterministic PNG LCD capture (simuCaptureArm / simuLcdNotify)
#   - analog override + --pipe steering channel (sdl_simu.cpp)
option(WIDGET_STUDIO "Widget Studio dev hooks" OFF)
if(WIDGET_STUDIO)
  message(STATUS "Widget Studio dev hooks: ON (simu target only)")
endif()

if(MINGW)
  # struct packing breaks on MinGW w/out -mno-ms-bitfields:
  #   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52991
  #   http://stackoverflow.com/questions/24015852/struct-packing-and-alignment-with-mingw
  # 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

if(ASAN AND NOT WASI)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

set(SIMU_DRIVERS
  simulib.cpp
  simufatfs.cpp
  simudisk.cpp
  simulcd.cpp
  audio_driver.cpp
  switch_driver.cpp
  adc_driver.cpp
  module_drivers.cpp
  led_driver.cpp
  backlight_driver.cpp
  bt_driver.cpp
  timers_driver.cpp
  abnormal_reboot.cpp
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Pack all simu driver sources into an object lib
add_library(simu_drivers OBJECT EXCLUDE_FROM_ALL
  ${SIMU_DRIVERS}
)

add_dependencies(simu_drivers simu_keys simu_switches hal_settings hal_pwm_sticks hal_keys hal_keys_lock hal_adc_inputs lua_inputs lua_mixsrc lua_keys yaml_inputs)

get_property(SIMU_SRC_OPTIONS
  TARGET radiolib_native
  PROPERTY INTERFACE_COMPILE_OPTIONS
)

target_compile_options(simu_drivers PRIVATE ${SIMU_SRC_OPTIONS})
set_property(TARGET simu_drivers PROPERTY POSITION_INDEPENDENT_CODE ON)

if(WIDGET_STUDIO AND NOT WASI)
  # Compile the Lua module and simulator state only when explicitly enabled.
  # The PNG backend remains in the interactive simu target below so
  # gtests-radio and wasi-module never pull in another stb implementation.
  target_sources(radiolib_native PRIVATE
    ${RADIO_SRC_DIR}/lua/api_simu.cpp
  )

  # Limit WIDGET_STUDIO to the sources that implement the hooks. This keeps
  # the default simulator's preprocessor surface and rebuild footprint intact.
  set_property(SOURCE
    ${RADIO_SRC_DIR}/lua/api_simu.cpp
    ${RADIO_SRC_DIR}/gui/colorlcd/LvglWrapper.cpp
    ${RADIO_SRC_DIR}/thirdparty/Lua/src/linit.c
    TARGET_DIRECTORY radiolib_native
    APPEND PROPERTY COMPILE_DEFINITIONS WIDGET_STUDIO
  )
  set_property(SOURCE simulib.cpp
    APPEND PROPERTY COMPILE_DEFINITIONS WIDGET_STUDIO
  )
endif()

if(SIMU_AUX)
  target_compile_definitions(simu_drivers PRIVATE -DSIMU_COM_PORT=${SIMU_COM_PORT})
endif()

# Replace SIMU_SRC with the objects from radiolib_native & simu_drivers
set(SIMU_SRC
  $<TARGET_OBJECTS:radiolib_native>
  $<TARGET_OBJECTS:simu_drivers>
)
set(SIMU_SRC ${SIMU_SRC} PARENT_SCOPE)

# Set the options as well in parent scope to be used by unit tests
set(SIMU_SRC_OPTIONS ${SIMU_SRC_OPTIONS} PARENT_SCOPE)

if(WASI)
  # strip debug info in Release builds
  list(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE
    -Wl,--strip-debug
  )

  set(WASI_COMPILE_OPTIONS
    -Os
    -fno-exceptions
    -fno-rtti
    -ffunction-sections
    -fdata-sections
    # setjmp / longjmp handling
    -mllvm -wasm-enable-sjlj
  )

  if(USE_LTO)
    list(APPEND WASI_COMPILE_OPTIONS -flto=thin)
  endif()

  target_compile_options(radiolib_native PUBLIC ${WASI_COMPILE_OPTIONS})
  target_compile_options(simu_drivers PUBLIC ${WASI_COMPILE_OPTIONS})

  add_executable(wasi-module EXCLUDE_FROM_ALL
    ${SIMU_SRC}
  )
  set_target_properties(wasi-module PROPERTIES
    OUTPUT_NAME "edgetx-${FLAVOUR}-simulator"
    SUFFIX ".wasm"
  )

  target_compile_options(wasi-module PUBLIC ${WASI_COMPILE_OPTIONS})

  target_link_options(wasi-module PRIVATE
    -Wl,--import-memory
    -Wl,--export-memory
    -Wl,--max-memory=2147483648
    -Wl,--gc-sections
    # enable runtime to check stack / heap
    -Wl,--export=__heap_base,--export=__data_end
    -Wl,--export=malloc -Wl,--export=free
    # no main()
    -mexec-model=reactor
    # setjmp / longjmp
    -Wl,-mllvm,-wasm-enable-sjlj
    -lsetjmp
  )

  if(USE_LTO)
    target_link_options(wasi-module PRIVATE -Wl,--thinlto-jobs=all)
  endif()
else()
  add_executable(simu
    EXCLUDE_FROM_ALL
    ${SIMU_SRC}
    simuaudio.cpp
    display.cpp
    widgets.cpp
    knobs.cpp
    arg_parser.cpp
    sdl_simu.cpp
  )

  target_compile_options(simu PUBLIC -DSIMU)

  if(WIDGET_STUDIO)
    # The SDL main loop (sdl_simu.cpp) owns the --pipe steering channel and
    # the reset-request consumption; it lives in the simu executable target.
    # simu_capture.cpp (the stb_image_write PNG encoder) is here rather than
    # in simu_drivers for the same reason - see the comment above.
    target_sources(simu PRIVATE simu_capture.cpp)
    set_property(SOURCE
      arg_parser.cpp
      sdl_simu.cpp
      simu_capture.cpp
      APPEND PROPERTY COMPILE_DEFINITIONS WIDGET_STUDIO
    )
  endif()

  add_png_target(image_assets "assets/images/*.png")
  add_dependencies(simu image_assets)

  # STB
  set(STB_DIR ${RADIO_SRC_DIR}/thirdparty/stb)
  target_include_directories(simu PUBLIC ${STB_DIR})

  # ImGui
  include(FetchImgui)
  target_link_libraries(simu PRIVATE imgui)
endif()
