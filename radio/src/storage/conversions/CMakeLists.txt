set(STORAGE_CONVERSIONS 219)

if(PCB STREQUAL X7)
  if(PCBREV STREQUAL ZORRO OR PCBREV STREQUAL TPRO OR PCBREV STREQUAL TX12MK2)
    set(STORAGE_CONVERSIONS 221)
  endif()
endif()
message("Storage conversion: ${STORAGE_CONVERSIONS}")

if(STORAGE_CONVERSIONS)
  add_definitions(-DSTORAGE_CONVERSIONS=${STORAGE_CONVERSIONS})
  set(SRC
    ${SRC}
    storage/conversions/conversions.cpp
    storage/conversions/conversions_219_220.cpp
    storage/conversions/conversions_220_221.cpp
    )

  include(storage/conversions/yaml/CMakeLists.txt)
endif()
