file(GLOB translations-new *.py)

foreach(translation ${translations-new})
  get_filename_component(language ${translation} NAME_WE)
  get_filename_component(path ${translation} DIRECTORY)
  set(target ${language}.cpp)
  if(PYTHONINTERP_FOUND)
    add_custom_command(
      OUTPUT ${target}
      COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/encode-translation-new.py ${translation} ${target} ${language}
      DEPENDS ${translation} ${TOOLS_DIR}/encode-translation-new.py ${TOOLS_DIR}/charset.py
      )
  endif()
  list(APPEND translations_files ${target})
endforeach()

add_custom_target(radio_translations_new DEPENDS ${translations_files})

