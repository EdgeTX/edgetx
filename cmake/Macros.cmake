find_package(Git)

macro(git_id RESULT)
  set(${RESULT} 0)
  if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND "${GIT_EXECUTABLE}" "--git-dir=${PROJECT_SOURCE_DIR}/.git" "rev-parse" "--short=8" "HEAD" OUTPUT_VARIABLE proc_out ERROR_VARIABLE proc_err)
    if(proc_err)
      message(WARNING "Git failed with error: ${proc_err}")
    else()
      string(REGEX REPLACE "(.*)\n" "\\1" ${RESULT} ${proc_out})
    endif()
  elseif(NOT GIT_FOUND)
    message(WARNING "Git executable not found, revision number not available.")
  else()
    message(STATUS "Git repository not found, revision number not available.")
  endif()
endmacro(git_id)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
  endif ()
endmacro(use_cxx11)

macro(PrintTargetReport targetName)
  if(CMAKE_CXX_COMPILER MATCHES "/(clang-)?cl\\.exe$")
    set(cpp_version ${CMAKE_CXX_COMPILER_VERSION})
  else()
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE cpp_version)
  endif()
  if(cpp_version)
    string(STRIP "v${cpp_version}" cpp_version)
  else()
    set(cpp_version "WARNING: COMPILER NOT FOUND!")
  endif()
  message("TARGET ${targetName}: cpp compiler ${CMAKE_CXX_COMPILER} ${cpp_version}")

  if(VERBOSE_CMAKELISTS)
    get_directory_property(DirOpts COMPILE_OPTIONS)
    get_directory_property(DirDefs COMPILE_DEFINITIONS)
    string(REPLACE ";" " " DirOpts "${DirOpts}")
    string(REPLACE ";" "; " DirDefs "${DirDefs}")
    string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)
    message("\twith cpp flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${build_type}} ${DirOpts}")
    message("\twith link flags: ${CMAKE_EXE_LINKER_FLAGS}")
    message("\twith defs: ${DirDefs}")
    message("--------------")
  endif()
endmacro(PrintTargetReport)

function(AddCompilerFlags output)
  get_property(flags DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
  foreach(flag ${flags})
    set(ARGS ${ARGS} -D${flag})
  endforeach()

  get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  foreach(dir ${dirs})
    set(ARGS ${ARGS} -I${dir})
  endforeach()

  # Add hotfix for arm64
  set(ARGS ${ARGS} -Wno-asm-operand-widths -Wno-pragma-once-outside-header)

  set(${output} ${${output}} ${ARGS} PARENT_SCOPE)
endfunction()

function(GenerateDatacopy source output)

  set(GEN_DATACOPY ${RADIO_DIRECTORY}/util/generate_datacopy.py)
  set(GEN_DATACOPY_DEPEND ${CMAKE_CURRENT_SOURCE_DIR}/${source} ${GEN_DATACOPY})

  # Fetch defines / include directories in use
  AddCompilerFlags(GEN_DATACOPY_ARGS)

  set(GEN_DATACOPY_ARGS
    # source file MUST be the first argument
    ${CMAKE_CURRENT_SOURCE_DIR}/${source}
    -DBACKUP ${GEN_DATACOPY_ARGS} ${SYSROOT_ARG})

  set(GEN_DATACOPY_CMD
    ${PYTHON_EXECUTABLE} ${GEN_DATACOPY} ${GEN_DATACOPY_ARGS})

  add_custom_command(
    OUTPUT ${output}
    COMMAND ${GEN_DATACOPY_CMD} > ${output}
    DEPENDS ${GEN_DATACOPY_DEPEND}
    )

endfunction()

macro(AddHeadersSources)
  foreach(name
    ${${PROJECT_NAME}_NAMES})

    set(${PROJECT_NAME}_SRCS
      ${${PROJECT_NAME}_SRCS}
      ${name}.cpp
    )

    set(${PROJECT_NAME}_HDRS
      ${${PROJECT_NAME}_HDRS}
      ${name}.h
    )
  endforeach()
endmacro()
