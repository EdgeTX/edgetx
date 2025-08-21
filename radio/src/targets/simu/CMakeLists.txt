
option(SIMU_TARGET "Configure libsimulator/simu targets" ON)
if(NOT SIMU_TARGET)
  message(STATUS "libsimulator/simu targets disabled")
  return()
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
  gyro_driver.cpp
  bt_driver.cpp
  timers_driver.cpp
  abnormal_reboot.cpp
)

set(HW_DESC_JSON ${FLAVOUR}.json)
AddHWGenTarget(${HW_DESC_JSON} simu_keys simu_keys.inc)
AddHWGenTarget(${HW_DESC_JSON} simu_switches simu_switches.inc)
AddHWGenTarget(${HW_DESC_JSON} hal_adc_inputs hal_adc_inputs.inc)

set(SIMU_DRIVERS ${SIMU_DRIVERS}
  ${CMAKE_CURRENT_BINARY_DIR}/simu_keys.inc
  ${CMAKE_CURRENT_BINARY_DIR}/simu_switches.inc
  ${CMAKE_CURRENT_BINARY_DIR}/hal_adc_inputs.inc
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Pack all simu driver sources into an object lib
add_library(simu_drivers OBJECT EXCLUDE_FROM_ALL
  ${SIMU_DRIVERS}
)

get_property(SIMU_SRC_OPTIONS
  TARGET radiolib_native
  PROPERTY INTERFACE_COMPILE_OPTIONS
)

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

if(WASI)
  set(WASI_COMPILE_OPTIONS
    -Os
    -fno-exceptions
    -fno-rtti
    -ffunction-sections
    -fdata-sections
    # setjmp / longjmp handling
    -mllvm -wasm-enable-sjlj
  )

  target_compile_options(radiolib_native PUBLIC ${WASI_COMPILE_OPTIONS})
  target_compile_options(simu_drivers PUBLIC ${WASI_COMPILE_OPTIONS})

  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--strip-debug")

  add_executable(wasi-module EXCLUDE_FROM_ALL
    ${SIMU_SRC}
  )
  set_target_properties(wasi-module PROPERTIES SUFFIX ".wasm")

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

  add_png_target(image_assets "assets/images/*.png")
  add_dependencies(simu image_assets)

  # STB
  set(STB_DIR ${RADIO_SRC_DIR}/thirdparty/stb)
  target_include_directories(simu PUBLIC ${STB_DIR})

  # ImGui
  include(FetchImgui)
  target_link_libraries(simu PRIVATE imgui)
endif()
