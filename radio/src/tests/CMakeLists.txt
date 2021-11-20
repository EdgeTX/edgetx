

if(GTEST_INCDIR AND GTEST_SRCDIR AND Qt5Widgets_FOUND)
  add_library(gtests-radio-lib STATIC EXCLUDE_FROM_ALL ${GTEST_SRCDIR}/src/gtest-all.cc )
  target_include_directories(gtests-radio-lib PUBLIC ${GTEST_INCDIR} ${GTEST_INCDIR}/gtest ${GTEST_SRCDIR})
  add_definitions(-DSIMU -DGTESTS)
  remove_definitions(-DCLI)
  set(TESTS_PATH ${RADIO_SRC_DIR}/tests)
  set(TESTS_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${RADIO_SRC_DIR}/tests/location.h.in ${CMAKE_CURRENT_BINARY_DIR}/location.h @ONLY)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -O0 -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -O0 ${WARNING_FLAGS} -fsanitize=address")
  use_cxx11()  # ensure gnu++11 in CXX_FLAGS with CMake < 3.1

  if(PCB STREQUAL X12S OR PCB STREQUAL X10)

    set(EXTRACT_MODEL_FILES model_23_x12s model_23_x10 model_25_tx16s)

    foreach(model_file ${EXTRACT_MODEL_FILES})
      add_custom_command(
        DEPENDS ${TESTS_PATH}/${model_file}.otx
        OUTPUT ${TESTS_BUILD_PATH}/${model_file}/RADIO/radio.bin
        COMMAND mkdir -p ${model_file} && cd ${model_file} && unzip -o -q -DD ${TESTS_PATH}/${model_file}.otx >/dev/null
        WORKING_DIRECTORY ${TESTS_BUILD_PATH}
      )
      add_custom_target(${model_file}_files
        DEPENDS ${TESTS_BUILD_PATH}/${model_file}/RADIO/radio.bin
      )
      set(HORUS_MODEL_FILES ${HORUS_MODEL_FILES} ${model_file}_files)
    endforeach()
  endif()
  
  if(WIN32)
    target_include_directories(gtests-radio-lib PUBLIC ${WIN_INCLUDE_DIRS})
    target_link_libraries(gtests-radio-lib PRIVATE ${WIN_LINK_LIBRARIES})
  endif(WIN32)

  if(SDL_FOUND AND SIMU_AUDIO)
    target_include_directories(gtests-radio-lib PUBLIC ${SDL_INCLUDE_DIR})
    target_link_libraries(gtests-radio-lib PRIVATE ${SDL_LIBRARY})
  endif()

  foreach(FILE ${SRC})
    set(RADIO_SRC ${RADIO_SRC} ../${FILE})
  endforeach()

  file(GLOB TEST_SRC_FILES ${RADIO_SRC_DIR}/tests/*.cpp)

  if(MINGW)
    # struct packing breaks on MinGW w/out -mno-ms-bitfields: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52991 & http://stackoverflow.com/questions/24015852/struct-packing-and-alignment-with-mingw
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
  endif()

  add_executable(gtests-radio EXCLUDE_FROM_ALL
    ${GTEST_SRC}
    ${TEST_SRC_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/location.h
    ${RADIO_SRC}
    ../targets/simu/simpgmspace.cpp
    ../targets/simu/simueeprom.cpp
    ../targets/simu/simufatfs.cpp
    ../targets/simu/simulcd.cpp
    ../targets/simu/module_drivers.cpp
    ../targets/simu/led_driver.cpp
    ../targets/simu/backlight_driver.cpp
    )
  add_dependencies(gtests-radio ${RADIO_DEPENDENCIES} ${FIRMWARE_DEPENDENCIES} gtests-radio-lib)
  if(PCB STREQUAL X12S OR PCB STREQUAL X10)
    add_dependencies(gtests-radio ${HORUS_MODEL_FILES})
  endif()
  target_link_libraries(gtests-radio gtests-radio-lib pthread Qt5::Core Qt5::Widgets)
  message(STATUS "Added optional gtests target")
else()
  message(WARNING "WARNING: gtests target will not be available (check that GTEST_INCDIR, GTEST_SRCDIR, and Qt5Widgets are configured).")
endif()
