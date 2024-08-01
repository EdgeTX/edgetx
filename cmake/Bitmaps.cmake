macro(add_bitmaps_target targetname filter format args)
  set(bitmaps_files)
  file(GLOB bitmaps ${filter})
  foreach(bitmap ${bitmaps})
    get_filename_component(target ${bitmap} NAME_WE)
    set(target ${target}.lbm)
    add_custom_command(
      OUTPUT ${target}
      COMMAND ${PYTHON_EXECUTABLE} ${UTILS_DIR}/encode-bitmap.py --format ${format} ${args} ${bitmap} ${target}
      DEPENDS ${bitmap} ${UTILS_DIR}/encode-bitmap.py
    )
    list(APPEND bitmaps_files ${target})
  endforeach()
  add_custom_target(${targetname} DEPENDS ${bitmaps_files})
endmacro()

macro(add_png_target targetname filter)
  set(bitmaps_files)
  file(GLOB bitmaps ${filter})
  foreach(bitmap ${bitmaps})
    get_filename_component(target ${bitmap} NAME_WE)
    set(target ${target}.lbm)
    add_custom_command(
      OUTPUT ${target}
      COMMAND ${PYTHON_EXECUTABLE} ${UTILS_DIR}/bin2lbm.py ${bitmap} ${target}
      DEPENDS ${bitmap} ${UTILS_DIR}/bin2lbm.py
    )
    list(APPEND bitmaps_files ${target})
  endforeach()
  add_custom_target(${targetname} DEPENDS ${bitmaps_files})
endmacro()

macro(add_fonts_target targetname filter)
  set(fonts_files)
  file(GLOB fonts ${filter})
  foreach(font ${fonts})
    get_filename_component(target ${font} NAME_WE)
    set(target ${target}.lbm)
    add_custom_command(
      OUTPUT ${target}
      COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/encode-font.py ${font} ${target}
      DEPENDS ${font} ${TOOLS_DIR}/encode-font.py
    )
    list(APPEND fonts_files ${target})
  endforeach()
  add_custom_target(${targetname} DEPENDS ${fonts_files})
endmacro()

macro(add_truetype_font_target name size subset effect args)
  set(target font_${name})
  if(${effect} STREQUAL "bold")
    set(font ${FONT_BOLD})
  else()
    set(font ${FONT})
  endif()
  add_custom_command(
    OUTPUT ${target}.png ${target}.specs ${target}.lbm
    COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/build-font-bitmap.py --subset ${subset} --size ${size} --font ${font} --output ${target}
    COMMAND ${PYTHON_EXECUTABLE} ${UTILS_DIR}/bin2lbm.py ${target}.png ${target}.lbm
    DEPENDS ${TOOLS_DIR}/build-font-bitmap.py ${TOOLS_DIR}/charset.py ${UTILS_DIR}/encode-bitmap.py
  )
  add_custom_target(truetype_font_${name} DEPENDS ${target}.png ${target}.specs)
  set(truetype_fonts_targets ${truetype_fonts_targets} truetype_font_${name})
endmacro()
