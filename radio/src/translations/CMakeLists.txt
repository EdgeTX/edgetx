file(GLOB translations *.h.txt)

foreach(translation ${translations})
  get_filename_component(language ${translation} NAME_WE)
  get_filename_component(path ${translation} DIRECTORY)
  set(target ${language}.h)
  if(Python3_FOUND)
    add_custom_command(
      OUTPUT ${target}
      COMMAND ${Python3_EXECUTABLE} ${TOOLS_DIR}/encode-translation.py ${translation} ${target} ${language}
      DEPENDS ${translation} ${TOOLS_DIR}/encode-translation.py ${TOOLS_DIR}/charset.py
      )
  endif()
  list(APPEND translations_files ${target})
endforeach()

add_custom_target(radio_translations DEPENDS ${translations_files})
