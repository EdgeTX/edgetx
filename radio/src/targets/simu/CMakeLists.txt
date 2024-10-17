
option(SIMU_TARGET "Configure libsimulator/simu targets (can be turned off for compiling Companion only)" ON)
if(NOT SIMU_TARGET)
  message(STATUS "libsimulator/simu targets disabled")
  return()
endif()

set(SIMU_DRIVERS
  simpgmspace.cpp
  simueeprom.cpp
  simufatfs.cpp
  simudisk.cpp
  simulcd.cpp
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
  add_definitions(-DJOYSTICKS)
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
    ../../targets/common/arm/stm32/bootloader/bin_files.cpp
    ../../targets/common/arm/stm32/bootloader/boot.cpp
    ../../targets/${TARGET_DIR}/bootloader/boot_menu.cpp
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

if(Qt5Widgets_FOUND)
  set(SIMULATOR_FLAVOUR edgetx-${FLAVOUR})
  set(SIMULATOR_TARGET ${SIMULATOR_FLAVOUR}-simulator)

  add_definitions(-DSIMULATOR_FLAVOUR="${SIMULATOR_FLAVOUR}")
  include_directories(${COMPANION_SRC_DIRECTORY} ${COMPANION_SRC_DIRECTORY}/simulation)

  qt5_wrap_cpp(SIMULATOR_SRC
    ${COMPANION_SRC_DIRECTORY}/simulation/simulatorinterface.h
    opentxsimulator.h
    )
  list(APPEND SIMULATOR_SRC ${SIMU_SRC} opentxsimulator.cpp)
  add_library(${SIMULATOR_TARGET} SHARED ${SIMULATOR_SRC})

  target_compile_options(${SIMULATOR_TARGET} PRIVATE ${SIMU_SRC_OPTIONS})
  target_compile_definitions(${SIMULATOR_TARGET} PUBLIC ${APP_COMMON_DEFINES})
  target_link_libraries(${SIMULATOR_TARGET} ${SDL2_LIBRARIES} Qt5::Core Qt5::SerialPort)

  # Remove debug symbols on release builds
  if(NOT CLANG)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
      add_custom_command(TARGET ${SIMULATOR_TARGET} POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${SIMULATOR_TARGET}>)
    endif()
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

if(WIN32)
  include_directories(SYSTEM ${WIN_INCLUDE_DIRS})
  if(Qt5Widgets_FOUND)
    target_link_libraries(${SIMULATOR_TARGET} PRIVATE ${WIN_LINK_LIBRARIES})
  endif()
endif(WIN32)

if(MSVC)
  set(CMAKE_CXX_FLAGS "/EHsc")
  if(NOT CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /LD /MP")
  endif()
else()
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
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")


if(FOX_FOUND)
  add_executable(simu WIN32
    EXCLUDE_FROM_ALL
    ${SIMU_SRC}
    ${RADIO_SRC_DIR}/simu.cpp)

  target_include_directories(simu PUBLIC ${FOX_INCLUDE_DIR} )
  target_link_libraries(simu ${FOX_LIBRARY} pthread ${SDL2_LIBRARIES})
  target_compile_options(simu PRIVATE -DSIMU)
endif()

if(APPLE)
  # OS X compiler no longer automatically includes /Library/Frameworks in search path
  set(CMAKE_SHARED_LINKER_FLAGS -F/Library/Frameworks)

  set(SIMULATOR_BUNDLES)
  foreach(library ${OPENTX_LIBRARIES})
    set(SIMULATOR_BUNDLE "${library}-bundle")
    add_custom_target(${SIMULATOR_BUNDLE}
      COMMAND install_name_tool -change /opt/local/Library/Frameworks/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore lib${library}.dylib
      COMMAND install_name_tool -change /opt/local/Library/Frameworks/QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork lib${library}.dylib
      COMMAND install_name_tool -change /opt/local/Library/Frameworks/QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml lib${library}.dylib
      COMMAND install_name_tool -change /opt/local/Library/Frameworks/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui lib${library}.dylib
      COMMAND install_name_tool -change @rpath/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL lib${library}.dylib
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMAND pwd
      COMMAND cp lib${library}.dylib companion.app/Contents/Resources/
      DEPENDS ${library}
      )
    list(APPEND SIMULATOR_BUNDLES ${SIMULATOR_BUNDLE})
  endforeach()
  add_custom_target(opentx-simulators-bundle DEPENDS ${SIMULATOR_BUNDLES})
endif(APPLE)

PrintTargetReport("simu/libsimulator")
