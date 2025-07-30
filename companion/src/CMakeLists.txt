set(SIMULATOR_INSTALL_PREFIX "" CACHE STRING "Alternative simulator library search path")

set(COMPANION_NAME "companion")
set(SIMULATOR_NAME "simulator")
set(APP_NAME_SUFFIX "")

if(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|FreeBSD)")
  set(APP_NAME_SUFFIX "${VERSION_MAJOR}${VERSION_MINOR}")
  set(COMPANION_NAME "${COMPANION_NAME}${APP_NAME_SUFFIX}")
  set(SIMULATOR_NAME "${SIMULATOR_NAME}${APP_NAME_SUFFIX}")
  # by default cmake sets CMAKE_INSTALL_PREFIX to /usr/local however linuxdeploy works best with '/usr' as base directory
  if (${CMAKE_INSTALL_PREFIX} MATCHES "/usr/local")
    set(CMAKE_INSTALL_PREFIX "/usr")
  endif()
  if (NOT ${SIMULATOR_INSTALL_PREFIX} STREQUAL "")
    set(SIMULATOR_LIB_PATH ${SIMULATOR_INSTALL_PREFIX}/lib/${COMPANION_NAME})
  else()
    set(SIMULATOR_LIB_PATH "../lib/${COMPANION_NAME}")
  endif()
  message(STATUS "Simulators library search path: " ${SIMULATOR_LIB_PATH})
endif()

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

if(TARGET SDL2::SDL2)
  add_definitions(-DUSE_SDL)
  set(SDL2_LIBRARIES SDL2::SDL2)
endif()

add_definitions(${APP_COMMON_DEFINES})  # set in top-level CMakeLists

set(WARNING_FLAGS "${WARNING_FLAGS} -Wall")
if(WARNINGS_AS_ERRORS)
  set(WARNING_FLAGS "${WARNING_FLAGS} -Werror")
endif()

if(WIN32)
  if(NOT WIN_USE_CONSOLE)
    set(WIN_EXECUTABLE_TYPE WIN32)  # GUI (WinMain) app
  endif()
  if(MINGW)
    get_filename_component(MINGW_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    # struct packing breaks on MinGW w/out -mno-ms-bitfields: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52991
    # & http://stackoverflow.com/questions/24015852/struct-packing-and-alignment-with-mingw
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
  endif()
else()
  link_directories(/usr/local/lib)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${COMPANION_SRC_DIRECTORY}
  ${COMPANION_SRC_DIRECTORY}/shared
  ${SIMU_SRC_DIRECTORY}
  ${COMPANION_SRC_DIRECTORY}/thirdparty/qcustomplot
  ${COMPANION_SRC_DIRECTORY}/storage
)

############# Do macro replacements on input files #############

configure_file(${COMPANION_SRC_DIRECTORY}/version.h.in ${CMAKE_BINARY_DIR}/version.h @ONLY)

############# Translations ###############

### Generate .qm files and assemble resource (qrc) file.

# available Companion translations (src/translations/*.ts)
set(LANGUAGES cs da de en es fi fr he it ja ko nl pl pt ru sv zh_CN zh_TW)
foreach(language ${LANGUAGES})
  list(APPEND companion_TS translations/companion_${language}.ts)
endforeach(language)
# .ts -> .qm
qt_add_translation(companion_QM ${companion_TS})

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

### Add optional "companion_translations" target for updating .ts files
if(QT_LUPDATE_EXECUTABLE)
  add_custom_target(companion_translations
    WORKING_DIRECTORY ${COMPANION_SRC_DIRECTORY}
    COMMAND ${QT_LUPDATE_EXECUTABLE} ${COMPANION_SRC_DIRECTORY} -no-obsolete -ts ${companion_TS}
    )
else()
  message(STATUS "Qt lupdate not found, 'companion_translations' target will not be availabe.")
endif()

############# Import radio hardware definitions ###############
set(HWDEFS_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../radio/src")
set(HWDEFS_TMPL "${COMPANION_SRC_DIRECTORY}/hwdefs.qrc.in")
set(HWDEFS_PHDR "HWDEF_JSON_LIST")
set(HWDEFS_QRC "${CMAKE_CURRENT_BINARY_DIR}/hwdefs.qrc")
set(HWDEFS_CMD "${COMPANION_SRC_DIRECTORY}/../util/generate_hwdefs_qrc.py")

# qrc used by Companion and Simulator
add_custom_command(OUTPUT ${HWDEFS_QRC}
  COMMAND ${PYTHON_EXECUTABLE} ${HWDEFS_CMD}
      -d ${HWDEFS_DIR}
      -t ${HWDEFS_TMPL}
      -p ${HWDEFS_PHDR}
      -o ${HWDEFS_QRC}
  DEPENDS ${HWDEFS_TMPL}
)

############# Common lib ###############

set(common_SRCS
  customdebug.cpp
  helpers.cpp
  translations.cpp
  modeledit/node.cpp  # used in simulator
  modeledit/edge.cpp  # used by node
  )

set(common_MOC_HDRS
  helpers.h
  modeledit/node.h
  )

set(common_RESOURCES
  companion.qrc
  ${TRANSLATIONS_QRC}
  ${HWDEFS_QRC}
  )

qt_wrap_cpp(common_SRCS ${common_MOC_HDRS})
qt_add_resources(common_RCC ${common_RESOURCES})

add_library(common ${common_SRCS} ${common_RCC})

target_link_libraries(common
  datamodels
  firmwares
  shared
  simulation
  storage
  maxLibQt
  Qt::Core
  Qt::Xml
  Qt::Widgets
  Qt::SerialPort
  ${SDL2_LIBRARIES}
)

set(CPN_COMMON_LIB common)

############# Supporting libraries ###############

include(FetchMiniz)
include(FetchYamlCpp)
include(FetchMaxLibQt)

add_subdirectory(datamodels)
add_subdirectory(firmwares)
add_subdirectory(generaledit)
add_subdirectory(modeledit)
add_subdirectory(shared)
add_subdirectory(simulation)
add_subdirectory(storage)
add_subdirectory(thirdparty/qcustomplot)
add_subdirectory(updates)

############# Companion ###############

include_directories(${maxLibQt_SOURCE_DIR}/widgets)

set(companion_NAMES
  apppreferencesdialog
  burnconfigdialog
  comparedialog
  customizesplashdialog
  flashfirmwaredialog
  helpers_html
  labels
  logsdialog
  mainwindow
  mdichild
  modelprinter
  modelslist
  multimodelprinter
  printdialog
  process_copy
  process_flash
  process_sync
  profilechooser
  progressdialog
  progresswidget
  radiointerface
  radionotfound
  splashlibrarydialog
  styleeditdialog
  wizarddata
  wizarddialog
  dialogs/filesyncdialog
  )

foreach(name ${companion_NAMES})
  set(companion_SRCS
    ${companion_SRCS}
    ${name}.cpp
  )

  set(companion_HDRS
    ${companion_HDRS}
    ${name}.h
  )
endforeach()

set(companion_SRCS
  ${companion_SRCS}
  companion.cpp
)

set(companion_HDRS
  ${companion_HDRS}
  splashlabel.h
)

if(WIN32)
  set(icon_RC images/winicons/icon.rc)
endif()

add_executable(${COMPANION_NAME} MACOSX_BUNDLE ${WIN_EXECUTABLE_TYPE}
  ${companion_SRCS}
  ${companion_HDRS}
  ${icon_RC}
)

target_link_libraries(${COMPANION_NAME} PRIVATE
  ${CPN_COMMON_LIB}
  ${SDL2_LIBRARIES}
  generaledit
  modeledit
  qcustomplot
  miniz
  updates
)

PrintTargetReport("${COMPANION_NAME}")

############# Standalone simulator ###############

set(simu_SRCS simulator.cpp )

add_executable(${SIMULATOR_NAME} MACOSX_BUNDLE ${WIN_EXECUTABLE_TYPE}
  ${simu_SRCS}
  ${icon_RC}
)

target_link_libraries(${SIMULATOR_NAME} PRIVATE
  ${CPN_COMMON_LIB}
  ${SDL2_LIBRARIES}
)

add_subdirectory(tests)

############# Install ####################

# the current flavour is not automatically added if build in the current cmake iteration, so always
# add its library name to be sure
string(TOLOWER ${PCB} FLAVOUR)

if(PCBREV)
  if(PCB STREQUAL X7 AND PCBREV STREQUAL ACCESS)
    set(FLAVOUR x7access)
  elseif(PCB STREQUAL X9D+ AND PCBREV STREQUAL 2019)
    set(FLAVOUR x9d+2019)
  elseif(PCB STREQUAL X10 AND PCBREV STREQUAL EXPRESS)
    set(FLAVOUR x10express)
  else()
    string(TOLOWER ${PCBREV} FLAVOUR)
  endif()
endif(PCBREV)

set(COMPANION_TARGETS_DIR_ROOT "${COMPANION_SRC_DIRECTORY}/../targets")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(COMPANION_TARGETS_DIR "${COMPANION_TARGETS_DIR_ROOT}/linux")
elseif(APPLE)
  set(COMPANION_TARGETS_DIR "${COMPANION_TARGETS_DIR_ROOT}/mac")
elseif(WIN32)
  set(COMPANION_TARGETS_DIR "${COMPANION_TARGETS_DIR_ROOT}/windows")
endif()

# updated license file
set(LICENSE_FILE "${CMAKE_CURRENT_BINARY_DIR}/license.txt")
configure_file("${COMPANION_TARGETS_DIR_ROOT}/license.txt.in" ${LICENSE_FILE} @ONLY)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(STATUS "Install " ${CMAKE_BINARY_DIR} " to " ${CMAKE_INSTALL_PREFIX})
  install(TARGETS ${COMPANION_NAME}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
  install(TARGETS ${SIMULATOR_NAME}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})

  # configure and set variables used by package script
  set(COMPANION_DESKTOP_FILE ${CMAKE_CURRENT_BINARY_DIR}/${COMPANION_NAME}.desktop)
  configure_file(${COMPANION_TARGETS_DIR}/companion.desktop.in ${COMPANION_DESKTOP_FILE} @ONLY)

  set(SIMULATOR_DESKTOP_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SIMULATOR_NAME}.desktop)
  configure_file(${COMPANION_TARGETS_DIR}/simulator.desktop.in ${SIMULATOR_DESKTOP_FILE} @ONLY)

  # libsim*.so files
  install(DIRECTORY ${CMAKE_BINARY_DIR}/plugins/
          DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${COMPANION_NAME})

  # application icons
  set(icon_sizes "16;22;24;32;48;64;128;256;512")
  foreach(icon_size ${icon_sizes})
    install(FILES "images/linuxicons/${icon_size}x${icon_size}/companion.png"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/${icon_size}x${icon_size}/apps"
            RENAME "${COMPANION_NAME}.png")
  endforeach()

  unset(icon_size)
  unset(icon_sizes)

  install(FILES "images/linuxicons/scalable/companion.svg"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps"
          RENAME "${COMPANION_NAME}.svg")

  # udev rules
  install(FILES "${COMPANION_TARGETS_DIR}/45-companion-taranis.rules"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/udev/rules.d"
          RENAME "45-${COMPANION_NAME}-taranis.rules")

  install(FILES "${COMPANION_TARGETS_DIR}/45-usbasp.rules"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/udev/rules.d"
          RENAME "45-${COMPANION_NAME}-usbasp.rules")

  if(OPENSSL_FOUND)
    get_filename_component(OPENSSL_SSL_LIBRARY_DIR ${OPENSSL_SSL_LIBRARY} DIRECTORY)
    install(DIRECTORY ${OPENSSL_SSL_LIBRARY_DIR}/
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
            FILES_MATCHING
              PATTERN "libcrypto.so.*"
              PATTERN "libssl.so.*")
  endif()

  if(LIBUSB1_FOUND)
    get_filename_component(LIBUSB1_LIBRARY_DIR ${LIBUSB1_LIBRARY} DIRECTORY)
    install(DIRECTORY ${LIBUSB1_LIBRARY_DIR}/
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
            FILES_MATCHING PATTERN "libusb-1.0.so.*")
    if(DFU_UTIL_FOUND)
      # dfuutil is an app
      install(FILES "${DFU_UTIL_PATH}"
              DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
    endif()
  endif()

  set(OperatingSystem "Linux")
  # Shortcut target
  add_custom_target(companion DEPENDS ${COMPANION_NAME})
  add_custom_target(simulator DEPENDS ${SIMULATOR_NAME})

elseif(WIN32)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/_install")
  message(STATUS "Install to " ${CMAKE_INSTALL_PREFIX})

  # companion and simulator apps
  install(TARGETS
            ${COMPANION_NAME}
            ${SIMULATOR_NAME}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})

  # install libsim dlls into same directory as apps
  install(DIRECTORY ${CMAKE_BINARY_DIR}/plugins/
          DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
          COMPONENT Runtime)

  install(FILES ${LICENSE_FILE}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})

  # supporting utilities
  if(LIBUSB1_FOUND)
    cmake_path(GET LIBUSB1_LIBRARY
               PARENT_PATH
               LIBUSB1_LIBRARY_DIR)

    if(MINGW)
      # finds dll.a directory but we need dlls but should check extension just to be sure and future proof
      set(LIBUSB1_LIBRARY_DIR "${LIBUSB1_LIBRARY_DIR}/../bin")
    endif()

    set(LIBUSB1_LIBRARY_FILE "${LIBUSB1_LIBRARY_DIR}/libusb-1.0.dll")

    if(EXISTS "${LIBUSB1_LIBRARY_FILE}")
      install(FILES "${LIBUSB1_LIBRARY_FILE}"
              DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
              COMPONENT Runtime)
    endif()

    if(DFU_UTIL_FOUND)
      install(FILES "${DFU_UTIL_PATH}"
              DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
              COMPONENT Runtime)
    endif()
  endif(LIBUSB1_FOUND)

  if(OPENSSL_FOUND)
    cmake_path(GET OPENSSL_SSL_LIBRARY
               PARENT_PATH
               OPENSSL_SSL_LIBRARY_DIR)

    if(MINGW)
      # the dlls are stored in the bin directory
      set(OPENSSL_SSL_LIBRARY_DIR "${OPENSSL_SSL_LIBRARY_DIR}/../bin")
    endif()

    if(EXISTS "${OPENSSL_SSL_LIBRARY_DIR}")
      install(DIRECTORY ${OPENSSL_SSL_LIBRARY_DIR}/
              DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
              COMPONENT Runtime
              FILES_MATCHING
                PATTERN "libcrypto-*.dll"
                PATTERN "libssl-*.dll")

    endif()
  endif(OPENSSL_FOUND)

  if(TARGET SDL2::SDL2)
    # Get the SDL2 DLL location
    get_target_property(SDL2_DLL_PATH SDL2::SDL2 IMPORTED_LOCATION)
    if(SDL2_DLL_PATH)
      install(FILES "${SDL2_DLL_PATH}"
              DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
              COMPONENT Runtime)
    endif()
  endif()

elseif(APPLE)
  # Qt + Cmake + Mac is poorly documented. A lot of this is guesswork
  # and trial and error. Do not hesitate to fix it for the better
  set_target_properties(${COMPANION_NAME} PROPERTIES
      MACOSX_RPATH TRUE
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST ${COMPANION_TARGETS_DIR}/MacOSXBundleInfo.plist.in

      INSTALL_RPATH "@executable_path/../Frameworks"
      BUILD_WITH_INSTALL_RPATH TRUE
  )

  # This the name that the user will see in the generated DMG and what the application
  # will be called under /Applications. We include the version string to make installing
  # different versions side-by-side
  set(COMPANION_OSX_APP_BUNDLE_NAME "EdgeTX Companion ${VERSION_FAMILY}")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.edgetx.companion")

  set(companion_app_dir "companion.app")
  set(companion_res_dir "${companion_app_dir}/Contents/Resources")

  # Only Companion is distributed for macOS
  install(TARGETS ${COMPANION_NAME}
          BUNDLE DESTINATION . COMPONENT Runtime)

  # libsims
  install(DIRECTORY ${CMAKE_BINARY_DIR}/plugins/
          DESTINATION "${companion_res_dir}"
          COMPONENT Runtime
  )

  # menu file
  install(DIRECTORY ${COMPANION_TARGETS_DIR}/qt_menu.nib
          DESTINATION "${companion_res_dir}"
          COMPONENT Runtime
  )

  # Add icon
  set(MAC_ICON_FILE ${COMPANION_SRC_DIRECTORY}/images/iconmac.icns)
  set_source_files_properties(
    ${MAC_ICON_FILE}
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )

  install(FILES ${MAC_ICON_FILE}
          DESTINATION ${companion_res_dir}
          COMPONENT Runtime)

  if(DFU_UTIL_FOUND)
    # Copy dfu-util, resolve symlink first
    file(REAL_PATH "${DFU_UTIL_PATH}" DFU_UTIL_ABSOLUTE_PATH)
    install(PROGRAMS ${DFU_UTIL_ABSOLUTE_PATH}
            DESTINATION ${companion_res_dir}
            COMPONENT Runtime)
  endif(DFU_UTIL_FOUND)
endif()

############# Packaging ####################

string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWERCASE)

# Variables common to all CPack generators
set(CPACK_GENERATOR)
set(CPACK_PACKAGE_NAME "companion${APP_NAME_SUFFIX}")
string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Models and settings editor for the ${PROJECT_NAME} open source firmware")
set(CPACK_PACKAGE_VENDOR "${PROJECT_NAME}")
set(CPACK_PACKAGE_CONTACT "${PROJECT_NAME} Project Steering Committee <psc@${PROJECT_NAME_LOWERCASE}.org>")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://${PROJECT_NAME_LOWERCASE}.org")
set(CPACK_PACKAGE_VERSION "${VERSION_FAMILY}")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_REVISION})
set(CPACK_RESOURCE_FILE_LICENSE ${LICENSE_FILE})
# The file stripping is deliberately disabled, with the stripped file we get
# very poor trace-backs from the users when they report Companion crash
set(CPACK_STRIP_FILES FALSE)

if(WIN32)
  set(companion_executable_path "\${CMAKE_INSTALL_BINDIR}/$<TARGET_FILE_NAME:companion>")
  set(simulator_executable_path "\${CMAKE_INSTALL_BINDIR}/$<TARGET_FILE_NAME:simulator>")
  cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH MINGW_BIN_DIR)

  qt_generate_deploy_script(
    TARGET companion
    OUTPUT_SCRIPT qt_deploy_script
    CONTENT "
      set(QT_DEPLOY_PREFIX ${CMAKE_INSTALL_PREFIX})
      qt_deploy_runtime_dependencies(
        EXECUTABLE ${companion_executable_path}
        ADDITIONAL_EXECUTABLES ${simulator_executable_path}
        NO_TRANSLATIONS
        GENERATE_QT_CONF
        VERBOSE
      )
    "
  )

  # Create Windows installer with NSIS
  # TODO use in-built CPack NSIS
  find_program(NSIS_EXE makensis.exe
    PATHS
      "C:/Program Files/NSIS"
      "C:/Program Files (x86)/NSIS"
      "C:/Programs/NSIS"
      "${WIN_EXTRA_LIBS_PATH}/NSIS"
  )

  if(NSIS_EXE)
    # list(APPEND CPACK_GENERATOR "NSIS")
    # Set CPACK_NSIS_variables

    cmake_path(NATIVE_PATH CMAKE_INSTALL_PREFIX NORMALIZE NSIS_DISTRO)   # all files in here will be included in installer
    cmake_path(NATIVE_PATH COMPANION_TARGETS_DIR NORMALIZE NSIS_TARGETS_DIR)   # all files in here will be included in installer
    set(NSI_FILE "${PROJECT_BINARY_DIR}/companion/companion.nsi")
    configure_file("${COMPANION_TARGETS_DIR}/companion.nsi.in" "${NSI_FILE}" @ONLY)

    # The 'install' target prepares all the distro files, make sure it has been executed first.
    add_custom_target(installer
      COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target install --config Release
      COMMAND "${CMAKE_COMMAND}" -P "${qt_deploy_script}"
      # Hack to overwrite the pthread lib shipped with Qt
      COMMAND "${CMAKE_COMMAND}" -E copy "${MINGW_BIN_DIR}/libwinpthread-1.dll" "${CMAKE_BINARY_DIR}/_install/bin/libwinpthread-1.dll"
      COMMAND "${NSIS_EXE}" "${NSI_FILE}"
      DEPENDS "${NSI_FILE}" "${qt_deploy_script}"
      COMMENT "Building Windows NSIS installer..."
    )
  endif()

elseif(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_BINARY_DRAGNDROP ON)

  set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME} Companion")
  set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_LOWERCASE}-${CPACK_PACKAGE_NAME_LOWERCASE}-${VERSION}")
  set(CPACK_DMG_DS_STORE ${COMPANION_TARGETS_DIR}/DS_Store)

  qt_generate_deploy_script(
    TARGET companion
    OUTPUT_SCRIPT qt_deploy_companion
    CONTENT "
      qt_deploy_runtime_dependencies(
        EXECUTABLE ${companion_app_dir}
        ADDITIONAL_EXECUTABLES ${companion_res_dir}/dfu-util
        NO_TRANSLATIONS
        NO_COMPILER_RUNTIME
        GENERATE_QT_CONF
      )
    "
  )

  # Run macdeployqt
  install(SCRIPT ${qt_deploy_companion} COMPONENT Runtime)

  # Fix Code Signing and rename bundle
  install(CODE "
      set(app_dir \"\${CMAKE_INSTALL_PREFIX}/${companion_app_dir}\")
      execute_process(COMMAND \${CMAKE_COMMAND} -E echo \"Re-signing: \${app_dir}\")
      execute_process(
        COMMAND codesign --remove-signature \${app_dir}
        COMMAND codesign --force --deep -s - \${app_dir}
      )
      file(RENAME \"\${app_dir}\" \"\${CMAKE_INSTALL_PREFIX}/${COMPANION_OSX_APP_BUNDLE_NAME}.app\")
  " COMPONENT Runtime)

else()
  set(LINUXDEPLOY_APPIMAGE "linuxdeploy-x86_64.AppImage")
  set(LINUXDEPLOY_PLUGIN_QT "linuxdeploy-plugin-qt-x86_64.AppImage")
  set(LINUXDEPLOY_URL "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous")
  set(LINUXDEPLOY_PLUGIN_QT_URL "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous")
  set(LINUXDEPLOY_DIRECTORY "${CMAKE_BINARY_DIR}/linuxdeploy")
  set(LINUXDEPLOY_APP "${LINUXDEPLOY_DIRECTORY}/usr/bin/linuxdeploy")

  if(NOT EXISTS "${LINUXDEPLOY_DIRECTORY}")
    message(STATUS "Downloading linuxdeploy and plugins...")

    file(DOWNLOAD "${LINUXDEPLOY_URL}/${LINUXDEPLOY_APPIMAGE}" "${CMAKE_BINARY_DIR}/${LINUXDEPLOY_APPIMAGE}")
    file(DOWNLOAD "${LINUXDEPLOY_PLUGIN_QT_URL}/${LINUXDEPLOY_PLUGIN_QT}" "${CMAKE_BINARY_DIR}/${LINUXDEPLOY_PLUGIN_QT}")

    message(STATUS "Downloading linuxdeploy and plugins finished")

    # for security reasons by default AppImages are prohibited from running in Docker containers
    # so extract Appimage contents and run as normal apps

    file(GLOB linuxdeployfiles ${CMAKE_BINARY_DIR}/linuxdeploy*.AppImage)

    foreach(file ${linuxdeployfiles})
      execute_process(
        COMMAND chmod +x "${file}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

      execute_process(
        COMMAND "${file}" --appimage-extract
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

      file(REMOVE "${file}")
    endforeach()

    file(REMOVE_RECURSE "${LINUXDEPLOY_DIRECTORY}")

    execute_process(
      COMMAND mv "${CMAKE_BINARY_DIR}/squashfs-root" "${LINUXDEPLOY_DIRECTORY}"
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

    # test installation
    execute_process(
      COMMAND ${LINUXDEPLOY_APP} --list-plugins
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
  endif()

  list(APPEND CPACK_GENERATOR "External")
  set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}")
  set(APPIMAGE_DIR "${CPACK_PACKAGE_DIRECTORY}/_CPack_Packages/Linux/External/AppImage")
  configure_file("${COMPANION_TARGETS_DIR}/CPackLinuxDeploy.cmake.in" "${CMAKE_BINARY_DIR}/CPackExternal.cmake" @ONLY)
  set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_BINARY_DIR}/CPackExternal.cmake")
endif()

include(CPack)

