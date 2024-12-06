if(APPLE AND DEFINED ENV{HOMEBREW_PREFIX} AND EXISTS $ENV{HOMEBREW_PREFIX}/opt/qt@5)
  # If Homebrew is used, HOMEBREW_PREFIX should be defined
  list(APPEND CMAKE_PREFIX_PATH "$ENV{HOMEBREW_PREFIX}/opt/qt@5")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Xml)
find_package(Qt5LinguistTools)
find_package(Qt5PrintSupport)
find_package(Qt5Multimedia)
find_package(Qt5Svg)
find_package(Qt5SerialPort)

if(Qt5Core_FOUND)
  message(STATUS "Qt Version: ${Qt5Core_VERSION}")

  ### Get locations of Qt binary executables & libs (libs are for distros, not for linking)
  # first set up some hints
  get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
  get_filename_component(qt_core_path ${QtCore_LOCATION} PATH)
  if(APPLE)
    get_filename_component(qt_core_path "${qt_core_path}/.." ABSOLUTE)
  endif()

  set(QT_LIB_DIR ${qt_core_path} CACHE PATH "Path to Qt libraries (.dll|.framework|.so).")
  find_path(QT_BIN_DIR NAMES "qmake" "qmake.exe" HINTS "${CMAKE_PREFIX_PATH}/bin" "${qt_core_path}/../bin" "${qt_core_path}" DOC "Path to Qt binaries (qmake, lupdate, etc.).")
  find_program(QT_QMAKE_EXECUTABLE qmake HINTS "${QT_BIN_DIR}" DOC "Location of qmake program.")
  find_program(QT_LUPDATE_EXECUTABLE lupdate HINTS "${QT_BIN_DIR}" DOC "Location of Qt's 'lupdate' program for updating translation files.")

  # Try to find Qt translations
  if(QT_QMAKE_EXECUTABLE)
    execute_process(
      COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_TRANSLATIONS
      OUTPUT_VARIABLE qt_translations_hint
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
  endif()
  find_path(QT_TRANSLATIONS_DIR NAMES "qt_en.qm" HINTS "${qt_translations_hint}" DOC "Path to prebuilt Qt translations (qt_*.qm).")

  ### Common definitions for the Qt-based apps
  list(APPEND APP_COMMON_DEFINES -DSIMU)
  list(APPEND APP_COMMON_DEFINES -DQXT_STATIC)
  list(APPEND APP_COMMON_DEFINES -DQT_USE_QSTRINGBUILDER)  # more efficient QString construction using % operator

  if(MSVC)
    list(APPEND APP_COMMON_DEFINES -D__STDC_LIMIT_MACROS)
  endif()
  if(APP_CUSTOM_DBG_HANDLER)
    # provide full qDebug log context to our custom handler. This may also affect libsimulator, which is why it is here
    list(APPEND APP_COMMON_DEFINES -DQT_MESSAGELOGCONTEXT)
    list(APPEND APP_COMMON_DEFINES -DAPP_DBG_HANDLER_ENABLE=1)
    if(APP_CUSTOM_DBG_LEVEL GREATER -1 AND APP_CUSTOM_DBG_LEVEL LESS 5)
      list(APPEND APP_COMMON_DEFINES -DAPP_DBG_HANDLER_DEFAULT_LEVEL=${APP_CUSTOM_DBG_LEVEL})
    endif()
    if(NOT APP_CUSTOM_DBG_SHOW_FILE STREQUAL "DEFAULT" AND (APP_CUSTOM_DBG_SHOW_FILE STREQUAL "1" OR APP_CUSTOM_DBG_SHOW_FILE STREQUAL "0"))
      list(APPEND APP_COMMON_DEFINES -DAPP_DBG_HANDLER_SHOW_SRC_PATH=${APP_CUSTOM_DBG_SHOW_FILE})
    endif()
  else()
    list(APPEND APP_COMMON_DEFINES -DAPP_DBG_HANDLER_ENABLE=0)
  endif()
else()
  message(WARNING "Qt not found! Companion and Simulator builds disabled.")
endif()
