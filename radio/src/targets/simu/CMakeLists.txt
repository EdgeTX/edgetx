
option(SIMU_TARGET "Configure libsimulator/simu targets" ON)
if(NOT SIMU_TARGET)
  message(STATUS "libsimulator/simu targets disabled")
  return()
endif()

set(SIMU_DRIVERS
  simpgmspace.cpp
  simufatfs.cpp
  simudisk.cpp
  simulcd.cpp
  audio_driver.cpp
  switch_driver.cpp
  adc_driver.cpp
  module_drivers.cpp
  led_driver.cpp
  backlight_driver.cpp
  gyro_driver.cpp
  bt_driver.cpp
  timers_driver.cpp
  abnormal_reboot.cpp
  )

set(HW_DESC_JSON ${FLAVOUR}.json)
AddHardwareDefTarget(${HW_DESC_JSON})
AddHWGenTarget(${HW_DESC_JSON} simu_keys simu_keys.inc)
AddHWGenTarget(${HW_DESC_JSON} simu_switches simu_switches.inc)
AddHWGenTarget(${HW_DESC_JSON} hal_adc_inputs hal_adc_inputs.inc)

set(SIMU_DRIVERS ${SIMU_DRIVERS}
  ${CMAKE_CURRENT_BINARY_DIR}/simu_keys.inc
  ${CMAKE_CURRENT_BINARY_DIR}/simu_switches.inc
  ${CMAKE_CURRENT_BINARY_DIR}/hal_adc_inputs.inc
  ${CMAKE_CURRENT_BINARY_DIR}/${HW_DESC_JSON}
  )
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(SDL2_FOUND)
  include_directories(${SDL2_INCLUDE_DIRS})
  target_compile_options(radiolib_native PUBLIC -DSIMU_AUDIO)
  if(TARGET SDL2::SDL2)
    set(SDL2_LIBRARIES SDL2::SDL2)
  endif()
endif()

if(SIMU_LUA_COMPILER)
  add_definitions(-DLUA_COMPILER)
endif()

if(SIMU_BOOTLOADER)
  add_definitions(-DSIMU_BOOTLOADER)
  set(SIMU_DRIVERS ${SIMU_DRIVERS}
    ../../bootloader/bin_files.cpp
    ../../bootloader/boot.cpp
    # TODO - how to handle this for B&W (not currently used)?
    ../../gui/colorlcd/boot_menu.cpp
    ../../thirdparty/lz4/lz4.c
    )
endif()

# Pack all simu driver sources into an object lib
add_library(simu_drivers OBJECT EXCLUDE_FROM_ALL
  ${SIMU_DRIVERS})

get_property(SIMU_SRC_OPTIONS
  TARGET radiolib_native
  PROPERTY INTERFACE_COMPILE_OPTIONS)

target_compile_options(simu_drivers PRIVATE ${SIMU_SRC_OPTIONS})

set_property(TARGET simu_drivers PROPERTY POSITION_INDEPENDENT_CODE ON)

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

if(Qt6Widgets_FOUND)
  set(SIMULATOR_FLAVOUR edgetx-${FLAVOUR})
  set(SIMULATOR_TARGET ${SIMULATOR_FLAVOUR}-simulator)

  add_definitions(-DSIMULATOR_FLAVOUR="${SIMULATOR_FLAVOUR}")
  include_directories(${COMPANION_SRC_DIRECTORY} ${COMPANION_SRC_DIRECTORY}/simulation)

  qt_wrap_cpp(SIMULATOR_SRC
    ${COMPANION_SRC_DIRECTORY}/simulation/simulatorinterface.h
    opentxsimulator.h
  )
  list(APPEND SIMULATOR_SRC ${SIMU_SRC} opentxsimulator.cpp)

  add_library(${SIMULATOR_TARGET} SHARED ${SIMULATOR_SRC})
  if(SDL2_FOUND)
    target_sources(${SIMULATOR_TARGET} PRIVATE simuaudio.cpp)
  else()
    target_sources(${SIMULATOR_TARGET} PRIVATE no_audio.cpp)
  endif()

  target_compile_options(${SIMULATOR_TARGET} PRIVATE ${SIMU_SRC_OPTIONS})
  target_compile_definitions(${SIMULATOR_TARGET} PUBLIC ${APP_COMMON_DEFINES})
  target_link_libraries(${SIMULATOR_TARGET} ${SDL2_LIBRARIES} Qt::Core Qt::SerialPort)

  # Remove debug symbols on release builds
  if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    add_custom_command(TARGET ${SIMULATOR_TARGET} POST_BUILD
      COMMAND ${CMAKE_STRIP} -x $<TARGET_FILE:${SIMULATOR_TARGET}>)
  endif()

  add_custom_command(
    TARGET ${SIMULATOR_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${SIMULATOR_TARGET}> ${CMAKE_BINARY_DIR}/plugins/
  )

  # Introduced at 2.5 - TODO replace with a code change
  # When compiled shared simulators using libopenui and Companion have classes named MainWindow
  # When the simulator is launched from Companion the runtime dynamic linking can result in Companion overiding the simulator
  # and crashing the simulator under some OSes
  # The flag forces the runtime linker to always look for references in the simulator first
  if (NOT CLANG)
    target_link_options(${SIMULATOR_TARGET} PRIVATE "LINKER:-Bsymbolic")
  endif()
  #
  add_custom_target(libsimulator DEPENDS ${SIMULATOR_TARGET})

  # Prepare the "all-simu-libs" target to build simulator libraries for *every* supported PCB type (PCB_TYPES list)
  #  (a fast build machine or corresponding amount of patience is recommended for this target).
  if(${CMAKE_GENERATOR} MATCHES ".*Unix Makefiles$")
    # Ensure parallel builds with GCC make (try with MAKEFLAGS but this seems to be getting clobbered somewhere).
    # otherwise 'make' may complain: "warning: jobserver unavailable: using -j1..." and build single-threaded.  (Ninja always rocks this.)
    set(mkflags "$ENV{MAKEFLAGS}")
    if (mkflags STREQUAL "")
      set(mkflags "-j")
    endif()
  endif()
  set(all_libs_cmd ${CMAKE_COMMAND} -E echo "Building all simulator libraries")
  foreach(pcb ${PCB_TYPES})
    set(pcbrev "")
    if(pcb STREQUAL X12S)
      set(pcbrev "-DPCBREV=13")
    endif()
    set(all_libs_cmd ${all_libs_cmd} && ${CMAKE_COMMAND} -DPCB=${pcb} ${pcbrev} -Wno-dev ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} && ${CMAKE_COMMAND} --build . --target libsimulator -- ${mkflags})
  endforeach()
  add_custom_target(all-simu-libs COMMAND ${all_libs_cmd} USES_TERMINAL WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if(MINGW)
  # struct packing breaks on MinGW w/out -mno-ms-bitfields: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52991 & http://stackoverflow.com/questions/24015852/struct-packing-and-alignment-with-mingw
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

if(ASAN)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

add_executable(simu
  EXCLUDE_FROM_ALL
  ${SIMU_SRC}
  simuaudio.cpp
  display.cpp
  widgets.cpp
  knobs.cpp
  sdl_simu.cpp
)

include(FetchImgui)
target_compile_options(simu PUBLIC -DSIMU)

add_png_target(image_assets "assets/images/*.png")
add_dependencies(simu image_assets)

set(STB_DIR ${RADIO_SRC_DIR}/thirdparty/stb)
target_include_directories(simu PUBLIC ${STB_DIR})
target_link_libraries(simu PUBLIC imgui)

target_link_libraries(simu PUBLIC
  ${SDL2_LIBRARIES}
)

PrintTargetReport("simu/libsimulator")
