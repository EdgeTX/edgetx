file(GLOB translations *.h.txt)

  if(FONTS_TARGET STREQUAL taranis_fonts)
    set(bwlcd T)
  elseif(FONTS_TARGET STREQUAL 9x_fonts_1bit)
    set(bwlcd T)
  else()
    set(bwlcd F)
  endif()

foreach(translation ${translations})
  get_filename_component(language ${translation} NAME_WE)
  get_filename_component(path ${translation} DIRECTORY)
  set(target ${language}.h)
  if(PYTHONINTERP_FOUND)
    add_custom_command(
      OUTPUT ${target}
      COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/encode-translation.py ${translation} ${target} ${language} ${bwlcd}
      DEPENDS ${translation} ${TOOLS_DIR}/encode-translation.py ${TOOLS_DIR}/charset.py
      )
  endif()
  list(APPEND translations_files ${target})
endforeach()

add_custom_target(radio_translations DEPENDS ${translations_files})
