set(SIMULATOR_INSTALL_PREFIX "" CACHE STRING "Alternative simulator library search path")

if(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|FreeBSD)")
  set(C9X_NAME_SUFFIX ${VERSION_MAJOR}${VERSION_MINOR})
  set(COMPANION_NAME "companion${C9X_NAME_SUFFIX}")
  set(SIMULATOR_NAME "simulator${C9X_NAME_SUFFIX}")
  if (NOT ${SIMULATOR_INSTALL_PREFIX} STREQUAL "")
    set(SIMULATOR_LIB_PATH ${SIMULATOR_INSTALL_PREFIX}/lib/companion${C9X_NAME_SUFFIX})
  else()
    set(SIMULATOR_LIB_PATH ${CMAKE_INSTALL_PREFIX}/lib/companion${C9X_NAME_SUFFIX})
  endif()
  message(STATUS "Simulators library search path: " ${SIMULATOR_LIB_PATH})
  set(SIMULATOR_LIB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/companion${C9X_NAME_SUFFIX})
else()
  set(COMPANION_NAME "companion")
  set(SIMULATOR_NAME "simulator")
endif()

# This the name that the user will see in the generated DMG and what the application
# will be called under /Applications. We include the version string to make installing
# different versions for different eeproms easier, i.e. without manually renaming
set(COMPANION_OSX_APP_BUNDLE_NAME "EdgeTX Companion ${VERSION_MAJOR}.${VERSION_MINOR}")

# On OS X we copy dfu-util to the application bundle. This the path from which we copy
# the binary (default homebrew path)
set(DFU_UTIL_PATH "/usr/local/bin/dfu-util")

option(ALLOW_NIGHTLY_BUILDS "Allow nightly builds download / update") # Disabled by default
if(ALLOW_NIGHTLY_BUILDS)
  add_definitions(-DALLOW_NIGHTLY_BUILDS)
endif(ALLOW_NIGHTLY_BUILDS)

option(DEBUG_STORAGE_IMPORT "Turn on debug output for storage import") # Disabled by default
if(DEBUG_STORAGE_IMPORT)
  add_definitions(-DDEBUG_STORAGE_IMPORT)
  message(STATUS "Storage import debugging enabled")
endif()
if(DEBUG)
  add_definitions(-DDEBUG)
endif()

option(TABLE_LAYOUT "Use QTableWidget for grid layouts") # Disabled by default
if(TABLE_LAYOUT)
  add_definitions(-DTABLE_LAYOUT)
  message(STATUS "Using QTableWidget")
endif()

if(WIN32)
  option(WIN_DO_FULL_INSTALL "'install' target will perform a full installation into CMAKE_INSTALL_PREFIX path, otherwise only supporting DLLs are copied to binaries folder (default)" OFF)
  option(WIN_USE_CONSOLE "Open a Windows command console for Simulator debug output." OFF)
  if(WIN_USE_CONSOLE)
    add_definitions(-DWIN_USE_CONSOLE_STDIO)
  endif()
endif()

if(SDL_FOUND)
  include_directories(${SDL_INCLUDE_DIR})
  add_definitions(-DJOYSTICKS)
endif()

add_definitions(${APP_COMMON_DEFINES})  # set in top-level CMakeLists

if(NOT MSVC)
  set(WARNING_FLAGS "${WARNING_FLAGS} -Wall")
  if(WARNINGS_AS_ERRORS)
    set(WARNING_FLAGS "${WARNING_FLAGS} -Werror")
  endif()
else()
  # silence lots of warnings regarding "insecure" use of functions (sprintf, strcpy, etc)
  set(WARNING_FLAGS "${WARNING_FLAGS} /D_CRT_SECURE_NO_WARNINGS")
  if(WARNINGS_AS_ERRORS)
    set(WARNING_FLAGS "${WARNING_FLAGS} /WX")
  endif()
endif()

if(WIN32)
  include_directories(SYSTEM ${WIN_INCLUDE_DIRS})
  if(NOT WIN_USE_CONSOLE)
    set(WIN_EXECUTABLE_TYPE WIN32)  # GUI (WinMain) app
  endif()
  if(MSVC)
    if(NOT CLANG)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /LD /MP")
    endif()
    if(NOT WIN_USE_CONSOLE)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.01")  # is this even needed?
    endif()
  elseif(MINGW)
    get_filename_component(MINGW_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    # struct packing breaks on MinGW w/out -mno-ms-bitfields: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52991 & http://stackoverflow.com/questions/24015852/struct-packing-and-alignment-with-mingw
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
  endif()
else()
  set(PTHREAD_LIBRARY pthread)
  link_directories(/usr/local/lib)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

use_cxx11()  # ensure gnu++11 in CXX_FLAGS with CMake < 3.1

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${COMPANION_SRC_DIRECTORY}
  ${COMPANION_SRC_DIRECTORY}/shared
  ${SIMU_SRC_DIRECTORY}
  ${COMPANION_SRC_DIRECTORY}/thirdparty/qcustomplot
  ${COMPANION_SRC_DIRECTORY}/thirdparty/miniz
  ${COMPANION_SRC_DIRECTORY}/storage
)

############# Do macro replacements on input files #############
configure_file(${COMPANION_SRC_DIRECTORY}/version.h.in ${CMAKE_BINARY_DIR}/version.h @ONLY)
configure_file(${COMPANION_SRC_DIRECTORY}/companion.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/companion.desktop @ONLY)
configure_file(${COMPANION_SRC_DIRECTORY}/simulator.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/simulator.desktop @ONLY)

############# Translations ###############

### Generate .qm files and assemble resource (qrc) file.

# available Companion translations (src/translations/*.ts)
set(LANGUAGES cs de en es fi fr it pl ru sv zh_CN ja)
foreach(language ${LANGUAGES})
  list(APPEND companion_TS translations/companion_${language}.ts)
endforeach(language)
# .ts -> .qm
qt5_add_translation(companion_QM ${companion_TS})

# add Qt translations if found
if(QT_TRANSLATIONS_DIR)
  # Some languages have all translations in one qt_*.qm file and others break them up into modules, in which case we need qt_base_*.qm
  file(GLOB qtbase_QM "${QT_TRANSLATIONS_DIR}/qtbase_*.qm")
  file(GLOB qt_QM "${QT_TRANSLATIONS_DIR}/qt_??.qm")  # don't want to match "qt_help_*.qm"
  list(APPEND qt_QM "${QT_TRANSLATIONS_DIR}/qt_zh_CN.qm" "${QT_TRANSLATIONS_DIR}/qt_zh_TW.qm")  # exceptions not matched by the GLOB
else()
  message(STATUS "QT_TRANSLATIONS_DIR not found, unable to package pre-built Qt translations with application.")
endif()

set(TRANSLATIONS_QRC "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc")

# Dynamically create translations.qrc file (XML) from all collected translation files.
foreach(qm_file ${companion_QM} ${qt_QM} ${qtbase_QM})
  get_filename_component(qm_file_name ${qm_file} NAME)
  # Add file with full path and file name (w/out path) as alias to be used in actual code
  set(TRANSLATION_FILES_LIST "${TRANSLATION_FILES_LIST}    <file alias=\"${qm_file_name}\">${qm_file}</file>\n")
endforeach()
configure_file(${COMPANION_SRC_DIRECTORY}/translations.qrc.in ${TRANSLATIONS_QRC} @ONLY)

### Add optional "translations" target for updating .ts files
if(QT_LUPDATE_EXECUTABLE)
  add_custom_target(companion_translations
    WORKING_DIRECTORY ${COMPANION_SRC_DIRECTORY}
    COMMAND ${QT_LUPDATE_EXECUTABLE} ${CMAKE_SOURCE_DIR} -no-obsolete -ts ${companion_TS}
    )
else()
  message(STATUS "Qt lupdate not found, 'translations' target will not be availabe.")
endif()

############# Common lib ###############

set(common_SRCS
  appdebugmessagehandler.cpp
  customdebug.cpp
  helpers.cpp
  translations.cpp
  modeledit/node.cpp  # used in simulator
  modeledit/edge.cpp  # used by node
  )

set(common_MOC_HDRS
  appdebugmessagehandler.h
  helpers.h
  modeledit/node.h
  )

set(common_RESOURCES
  companion.qrc
  ${TRANSLATIONS_QRC}
  )

qt5_wrap_cpp(common_SRCS ${common_MOC_HDRS})
qt5_add_resources(common_RCC ${common_RESOURCES})

add_library(common ${common_SRCS} ${common_RCC})

target_link_libraries(common
  firmwares
  storage
  simulation
  maxLibQtWidgets
  Qt5::Core
  Qt5::Xml
  Qt5::Widgets
  ${PTHREAD_LIBRARY}
  ${SDL_LIBRARY}
  ${WIN_LINK_LIBRARIES}
  )

set(CPN_COMMON_LIB common)

############# Supporting libraries ###############

add_subdirectory(datamodels)
add_subdirectory(firmwares)
add_subdirectory(shared)
add_subdirectory(modeledit)
add_subdirectory(generaledit)
add_subdirectory(simulation)
add_subdirectory(storage)
add_subdirectory(thirdparty/qcustomplot)
add_subdirectory(thirdparty/maxlibqt/src/widgets)

############# Companion ###############

set(companion_SRCS
  helpers_html.cpp
  mdichild.cpp
  modelslist.cpp
  apppreferencesdialog.cpp
  fwpreferencesdialog.cpp
  burnconfigdialog.cpp
  multimodelprinter.cpp
  comparedialog.cpp
  releasenotesdialog.cpp
  releasenotesfirmwaredialog.cpp
  customizesplashdialog.cpp
  radiointerface.cpp
  progresswidget.cpp
  progressdialog.cpp
  process_copy.cpp
  process_flash.cpp
  process_sync.cpp
  flashfirmwaredialog.cpp
  flasheepromdialog.cpp
  printdialog.cpp
  modelprinter.cpp
  logsdialog.cpp
  downloaddialog.cpp
  splashlibrarydialog.cpp
  mainwindow.cpp
  companion.cpp
  radionotfound.cpp
  wizarddata.cpp
  wizarddialog.cpp
  styleeditdialog.cpp
  dialogs/filesyncdialog.cpp         # TODO move to own lib with other dialogs
  profilechooser.cpp
  )

set(companion_MOC_HDRS
  apppreferencesdialog.h
  fwpreferencesdialog.h
  burnconfigdialog.h
  comparedialog.h
  printdialog.h
  logsdialog.h
  releasenotesdialog.h
  releasenotesfirmwaredialog.h
  customizesplashdialog.h
  splashlibrarydialog.h
  splashlabel.h
  progresswidget.h
  progressdialog.h
  process_copy.h
  process_flash.h
  process_sync.h
  flashfirmwaredialog.h
  flasheepromdialog.h
  downloaddialog.h
  mdichild.h
  mainwindow.h
  radionotfound.h
  wizarddialog.h
  modelprinter.h
  multimodelprinter.h
  modelslist.h
  styleeditdialog.h
  helpers_html.h
  dialogs/filesyncdialog.h
  profilechooser.h
  )

set(companion_UIS
  mdichild.ui
  comparedialog.ui
  logsdialog.ui
  apppreferencesdialog.ui
  fwpreferencesdialog.ui
  burnconfigdialog.ui
  downloaddialog.ui
  printdialog.ui
  htmldialog.ui
  customizesplashdialog.ui
  splashlibrarydialog.ui
  progresswidget.ui
  progressdialog.ui
  flashfirmwaredialog.ui
  flasheepromdialog.ui
  radionotfound.ui
  styleeditdialog.ui
  profilechooser.ui
  )

if(WIN32)
  set(icon_RC images/winicons/icon.rc)
endif()

qt5_wrap_ui(companion_SRCS ${companion_UIS})
qt5_wrap_cpp(companion_SRCS ${companion_MOC_HDRS})

add_executable(${COMPANION_NAME} MACOSX_BUNDLE ${WIN_EXECUTABLE_TYPE} ${companion_SRCS} ${icon_RC})
target_link_libraries(${COMPANION_NAME} PRIVATE generaledit modeledit qcustomplot shared ${CPN_COMMON_LIB})

PrintTargetReport("${COMPANION_NAME}")

############# Standalone simulator ###############

set(simu_SRCS simulator.cpp )

add_executable(${SIMULATOR_NAME} MACOSX_BUNDLE ${WIN_EXECUTABLE_TYPE} ${simu_SRCS} ${icon_RC})
target_link_libraries(${SIMULATOR_NAME} PRIVATE ${CPN_COMMON_LIB})

if(NOT MSVC)
  add_subdirectory(tests)
endif()

############# Install ####################

# Generate list of simulator plugins, used by all platforms
if(WIN32)
  file(GLOB simulator_plugins "${CMAKE_BINARY_DIR}/Release/*opentx-*simulator${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
  file(GLOB simulator_plugins "${CMAKE_BINARY_DIR}/*opentx-*simulator${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

# the current flavour is not automatically added if build in the current cmake iteration, so always
# add its library name to be sure
if(PCB STREQUAL X7 AND PCBREV STREQUAL ACCESS)
  set(FLAVOUR x7access)
elseif(PCB STREQUAL X7 AND PCBREV STREQUAL T12)
  set(FLAVOUR t12)
elseif(PCB STREQUAL X7 AND PCBREV STREQUAL TLITE)
  set(FLAVOUR tlite)
elseif(PCB STREQUAL X7 AND PCBREV STREQUAL TX12)
  set(FLAVOUR tx12)
elseif(PCB STREQUAL X7 AND PCBREV STREQUAL T8)
  set(FLAVOUR t8)
elseif(PCB STREQUAL X9D+ AND PCBREV STREQUAL 2019)
  set(FLAVOUR x9d+2019)
elseif(PCB STREQUAL X10 AND PCBREV STREQUAL EXPRESS)
  set(FLAVOUR x10express)
elseif(PCB STREQUAL X10 AND PCBREV STREQUAL T16)
  set(FLAVOUR t16)
elseif(PCB STREQUAL X10 AND PCBREV STREQUAL TX16S)
  set(FLAVOUR tx16s)
elseif(PCB STREQUAL X10 AND PCBREV STREQUAL T18)
  set(FLAVOUR t18)
else()
  string(TOLOWER ${PCB} FLAVOUR)
endif()
if(POLICY CMP0026)
  cmake_policy(SET CMP0026 OLD)  # https://cmake.org/cmake/help/v3.0/policy/CMP0026.html
endif()

if(SIMU_TARGET)
  get_property(current_plugin TARGET opentx-${FLAVOUR}-simulator PROPERTY LOCATION)
  list(APPEND simulator_plugins "${current_plugin}")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  message(STATUS "install " ${CMAKE_BINARY_DIR} " to " ${CMAKE_INSTALL_PREFIX}/bin)
  install(TARGETS ${COMPANION_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  install(TARGETS ${SIMULATOR_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  install(FILES ${simulator_plugins} DESTINATION "${SIMULATOR_LIB_INSTALL_PATH}")
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/companion.desktop DESTINATION share/applications RENAME companion${C9X_NAME_SUFFIX}.desktop)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/simulator.desktop DESTINATION share/applications RENAME simulator${C9X_NAME_SUFFIX}.desktop)
  if(${CMAKE_INSTALL_PREFIX} MATCHES "/usr/local")
    set(INSTALL_TEMP_SHR_PFX "/usr/")
    set(INSTALL_TEMP_LIB_PFX "/")
  endif()
  install(FILES images/linuxicons/16x16/companion.png DESTINATION ${INSTALL_TEMP_SHR_PFX}share/icons/hicolor/16x16/apps RENAME companion${C9X_NAME_SUFFIX}.png)
  install(FILES images/linuxicons/22x22/companion.png DESTINATION ${INSTALL_TEMP_SHR_PFX}share/icons/hicolor/22x22/apps RENAME companion${C9X_NAME_SUFFIX}.png)
  install(FILES images/linuxicons/24x24/companion.png DESTINATION ${INSTALL_TEMP_SHR_PFX}share/icons/hicolor/24x24/apps RENAME companion${C9X_NAME_SUFFIX}.png)
  install(FILES images/linuxicons/32x32/companion.png DESTINATION ${INSTALL_TEMP_SHR_PFX}share/icons/hicolor/32x32/apps RENAME companion${C9X_NAME_SUFFIX}.png)
  install(FILES images/linuxicons/48x48/companion.png DESTINATION ${INSTALL_TEMP_SHR_PFX}share/icons/hicolor/48x48/apps RENAME companion${C9X_NAME_SUFFIX}.png)
  install(FILES images/linuxicons/128x128/companion.png DESTINATION ${INSTALL_TEMP_SHR_PFX}share/icons/hicolor/128x128/apps RENAME companion${C9X_NAME_SUFFIX}.png)
  install(FILES images/linuxicons/256x256/companion.png DESTINATION ${INSTALL_TEMP_SHR_PFX}share/icons/hicolor/256x256/apps RENAME companion${C9X_NAME_SUFFIX}.png)
  install(FILES images/linuxicons/512x512/companion.png DESTINATION ${INSTALL_TEMP_SHR_PFX}share/icons/hicolor/512x512/apps RENAME companion${C9X_NAME_SUFFIX}.png)
  install(FILES images/linuxicons/scalable/companion.svg DESTINATION ${INSTALL_TEMP_SHR_PFX}share/icons/hicolor/scalable/apps RENAME companion${C9X_NAME_SUFFIX}.svg)
  install(FILES ../targets/linux/45-companion-taranis.rules DESTINATION ${INSTALL_TEMP_LIB_PFX}lib/udev/rules.d RENAME 45-companion${C9X_NAME_SUFFIX}-taranis.rules)
  install(FILES ../targets/linux/45-usbasp.rules DESTINATION ${INSTALL_TEMP_LIB_PFX}lib/udev/rules.d RENAME 45-companion${C9X_NAME_SUFFIX}-usbasp.rules)
  # Linux specific code
  set(OperatingSystem "Linux")
elseif(WIN32)
  if(NOT WIN_DO_FULL_INSTALL)
    # Just copy supporting DLLs to build folder
    set(INSTALL_DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    message(STATUS "Partial install to " ${INSTALL_DESTINATION})
  else()
    # Full Windows installation with all supporting files
    set(INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}")
    message(STATUS "Full install to " ${INSTALL_DESTINATION})
    # companion & simulator binaries
    install(TARGETS ${COMPANION_NAME} DESTINATION ${INSTALL_DESTINATION})
    install(TARGETS ${SIMULATOR_NAME} DESTINATION ${INSTALL_DESTINATION})
    install(FILES ${simulator_plugins} DESTINATION "${INSTALL_DESTINATION}")
    # supporting utilities
    set(INSTALL_TEMP_FILES dfu-util.exe libusb0.dll libusb-1.0.dll license.txt)
    foreach(tmpfile ${INSTALL_TEMP_FILES})
      install(FILES "${COMPANION_SRC_DIRECTORY}/../targets/windows/${tmpfile}" DESTINATION ${INSTALL_DESTINATION})
    endforeach()
  endif()  # WIN_DO_FULL_INSTALL

  # Qt dlls (using windeployqt)
  ## Qt translations are already packaged into our custom qrc. Also don't need software rendering engines.
  set(wdqtopts --no-translations --no-opengl-sw --no-system-d3d-compiler --no-angle)
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(wdqtopts ${wdqtopts} --release)  # release builds aren't always properly detected with mingw
  endif()
  if(MSVC)
    set(wdqtopts ${wdqtopts} --no-compiler-runtime)  # --compiler-runtime includes vcredist_x86, but we just copy the required DLLs manually
  endif()
  set(wdqtopts ${wdqtopts} -dir "\"${INSTALL_DESTINATION}\"" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/${COMPANION_NAME}.exe" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/${SIMULATOR_NAME}.exe")

  # Add installer command to execute windeployqt
  message(STATUS "windeployqt command: ${QT_BIN_DIR}/windeployqt.exe ${wdqtopts}")
  install(CODE "execute_process(COMMAND \"${QT_BIN_DIR}/windeployqt.exe\" ${wdqtopts})")

  # SDL dll
  if(SDL_FOUND AND EXISTS "${SDL_LIBRARY_PATH}/SDL.dll")
    set(SDL_DIR ${SDL_LIBRARY_PATH})  # this is also used by NSIS installer script
    install(FILES ${SDL_DIR}/SDL.dll DESTINATION ${INSTALL_DESTINATION})
  elseif(SDL_FOUND)
    message(WARNING "Installer: SDL.dll not found, set SDL_LIBRARY_PATH manually.")
  endif()

  # libeay (SSL) dlls  (used by QtNetwork) -- not strictly required but avoids possible issues with downloads and Qt warnings.
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" WIN_PROGRAM_FILES)
  find_path(EAY_LIB_DIR libeay32.dll HINTS
    "${WIN_PROGRAM_FILES}/OpenSSL" "${WIN_PROGRAM_FILES}/OpenSSL-Win32"
    "C:/OpenSSL/" "C:/OpenSSL-Win32/"
    "${WIN_EXTRA_LIBS_PATH}" "${WIN_EXTRA_LIBS_PATH}/OpenSSL" "${WIN_EXTRA_LIBS_PATH}/OpenSSL-Win32"
    "${MINGW_DIR}/../opt/bin"
    "${PYTHON_DIRECTORY}/lib/site-packages/PyQt4"
    PATH_SUFFIXES bin
    DOC "Path to libeay32.dll and ssleay32.dll (OpenSSL)"
  )
  if (EAY_LIB_DIR)
    install(FILES "${EAY_LIB_DIR}/libeay32.dll" "${EAY_LIB_DIR}/ssleay32.dll" DESTINATION ${INSTALL_DESTINATION} OPTIONAL)
  else()
    message(WARNING "Installer: SSL libs not found, set EAY_LIB_DIR manually.")
  endif()

  # C++/system dlls, depends on compiler
  if(MSVC)
    file(TO_CMAKE_PATH "$ENV{windir}/SysWOW64" WIN_SYSDIR)    # This would be "system32" for a 64-bit build or on x86... but how to tell?
    if (NOT EXISTS "${WIN_SYSDIR}")
      file(TO_CMAKE_PATH "$ENV{windir}/system32" WIN_SYSDIR)  # Must be 32-bit Windows.
    endif()
    install(FILES "${WIN_SYSDIR}/msvcp140.dll" "${WIN_SYSDIR}/vcruntime140.dll" DESTINATION ${INSTALL_DESTINATION} OPTIONAL)
    if(WIN_PTHREAD_DLL)
      install(FILES "${WIN_PTHREAD_DLL}" DESTINATION ${INSTALL_DESTINATION})
    endif()
  endif()

endif() # WIN32 install

############# Packaging ####################

# Create Windows installer with NSIS
if(WIN32)
  find_program(NSIS_EXE makensis.exe PATHS
    "C:/Program Files/NSIS"
    "C:/Program Files (x86)/NSIS"
    "C:/Programs/NSIS"
    "${WIN_EXTRA_LIBS_PATH}/NSIS"
  )

  if (NSIS_EXE)
    file(TO_NATIVE_PATH "${INSTALL_DESTINATION}" NSIS_DISTRO)  # all files in here will be included in installer
    set(NSI_FILE "${PROJECT_BINARY_DIR}/companion/companion.nsi")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../targets/windows/companion.nsi.in" "${NSI_FILE}" @ONLY)

    add_custom_target(installer
      # The 'install' target prepares all the distro files, make sure it has been executed first.
      COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target install --config Release
      COMMAND "${NSIS_EXE}" "${NSI_FILE}"
      DEPENDS "${NSI_FILE}"
      COMMENT "Building Windows NSIS installer..."
    )
  endif()
endif()

set(CPACK_PACKAGE_NAME "companion${C9X_NAME_SUFFIX}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Models and settings editor for the EdgeTX open source firmware")
string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)

# The file stripping is deliberately disabled, with the stripped file we get
# very poor trace-backs from the users when they report Companion crash
set(CPACK_STRIP_FILES FALSE)

# Qt5 + Cmake + Mac is poorly documented. A lot of this is guesswork
# and trial and error. Do not hesitate to fix it for the better
IF(APPLE)
  set(plugin_dest_dir ${COMPANION_NAME}.app/Contents/Plugins)
  set(qtconf_dest_dir ${COMPANION_NAME}.app/Contents/Resources)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/${COMPANION_NAME}.app")
  set(companion_res_dir ${COMPANION_NAME}.app/Contents/Resources)

  set_target_properties(${COMPANION_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Companion ${VERSION_MAJOR}.${VERSION_MINOR}")
  set_target_properties(${SIMULATOR_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Simulator ${VERSION_MAJOR}.${VERSION_MINOR}")

  # Use a non standard Info.plist that adds Retina support flags
  set_target_properties(${COMPANION_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/companion/targets/mac/MacOSXBundleInfo.plist.in)
  set_target_properties(${SIMULATOR_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/companion/targets/mac/MacOSXBundleInfo.plist.in)

  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.edgetx.companion")

  INSTALL(TARGETS ${COMPANION_NAME}
      BUNDLE DESTINATION . COMPONENT Runtime
      RUNTIME DESTINATION bin COMPONENT Runtime
      )

  install(FILES ${simulator_plugins} DESTINATION "${companion_res_dir}" COMPONENT Runtime)

  # Write qt.conf to tell qt where to find it plugins
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    "[Paths]\nPlugins = Plugins\n")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    DESTINATION "${companion_res_dir}"
    COMPONENT Runtime)

  # menu file
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/companion/targets/mac/qt_menu.nib DESTINATION "${companion_res_dir}" COMPONENT Runtime)

  # manually add the required plugins
  foreach (plugin ${Qt5Multimedia_PLUGINS} ${Qt5PrintSupport_PLUGINS} ${Qt5Gui_PLUGINS} ${Qt5Svg_PLUGINS})
    get_target_property(QPA_PLUGIN ${plugin} LOCATION)
    get_filename_component(QPA_PLUGIN_FILENAME ${QPA_PLUGIN} NAME)
    # Todo is there a nicer way to get the QT Plugin directory name?!
    get_filename_component(QPA_PLUGIN_DIRECTORY ${QPA_PLUGIN} DIRECTORY)
    get_filename_component(QPA_PLUGIN_DIRECTORY ${QPA_PLUGIN_DIRECTORY} NAME)

    install(FILES ${QPA_PLUGIN} DESTINATION "${plugin_dest_dir}/${QPA_PLUGIN_DIRECTORY}/" COMPONENT Runtime)

    list(APPEND bundle_qt_libs "\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/${QPA_PLUGIN_DIRECTORY}/${QPA_PLUGIN_FILENAME}")
  endforeach()

  # Add icon
  set(MACOSX_BUNDLE_ICON_FILE iconmac.icns)
  set(MAC_ICON_FILE ${COMPANION_SRC_DIRECTORY}/images/${MACOSX_BUNDLE_ICON_FILE})
  SET_SOURCE_FILES_PROPERTIES(${MAC_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set(${COMPANION_SOURCES} ${COMPANION_SOURCES} ${PROJECT_SOURCE_DIR}/images/${MACOSX_BUNDLE_ICON_FILE})
  install(FILES ${MAC_ICON_FILE} DESTINATION ${companion_res_dir} COMPONENT Runtime)

  # Copy dfu-util, resolve symlink first
  get_filename_component(DFU_UTIL_ABSOLUTE_PATH ${DFU_UTIL_PATH} REALPATH)
  install(PROGRAMS ${DFU_UTIL_ABSOLUTE_PATH} DESTINATION ${companion_res_dir} COMPONENT Runtime)

  set(bundle_tools_path "\${CMAKE_INSTALL_PREFIX}/${companion_res_dir}/dfu-util;")


  # Include depencies (adding frameworks, fixing the embbeded libraries)
  # I get write errors without setting BU_CHMOD_BUNDLE_ITEMS even though it is
  # technically a hack (that is already is in the Bundle library ...)
  install(CODE "
   include(BundleUtilities)
   file(GLOB bundle_simulator_libs \"\${CMAKE_INSTALL_PREFIX}/${companion_res_dir}/libopentx-*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
   set(BU_CHMOD_BUNDLE_ITEMS on)
   fixup_bundle(\"${APPS}\"   \"\${bundle_simulator_libs};${bundle_qt_libs};${bundle_tools_path}\"   \"${QT_LIB_DIR}\")
   file(RENAME \"\${CMAKE_INSTALL_PREFIX}/${COMPANION_NAME}.app\" \"\${CMAKE_INSTALL_PREFIX}/${COMPANION_OSX_APP_BUNDLE_NAME}.app\")
   " COMPONENT Runtime)
endif()

find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")

if(DPKG_PROGRAM)
  set(CPACK_GENERATOR "DEB")
  execute_process(
    COMMAND ${DPKG_PROGRAM} --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Bertrand Songis <bsongis-nosp@m-gmail.com>")
  set(CPACK_DEBIAN_PACKAGE_VERSION ${VERSION})
  set(CPACK_DEBIAN_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
  set(CPACK_DEBIAN_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
  set(CPACK_DEBIAN_ARCHITECTURE $(CMAKE_SYSTEM_PROCESSOR))
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5multimedia5-plugins")
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
elseif(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
# set(CPACK_GENERATOR "TGZ")  # for quick testing
  set(CPACK_BINARY_DRAGNDROP ON)
  set(CPACK_DMG_BACKGROUND_IMAGE ${COMPANION_SRC_DIRECTORY}/images/splash_dmg.png)
  set(CPACK_DMG_VOLUME_NAME "EdgeTX Companion")
  set(CPACK_DMG_DS_STORE ${PROJECT_SOURCE_DIR}/companion/targets/mac/DS_Store)
  set(CPACK_PACKAGE_FILE_NAME "opentx-${CPACK_PACKAGE_NAME_LOWERCASE}-${VERSION}")
else(DPKG_PROGRAM)
  set(CPACK_GENERATOR "RPM")
  set(CPACK_RPM_PACKAGE_VERSION ${VERSION})
  set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}-${VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
  set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/lib /lib/udev /lib/udev/rules.d")
endif(DPKG_PROGRAM)

include(CPack)
