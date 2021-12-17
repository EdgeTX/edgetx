set(STORAGE_CONVERSIONS 219)

if(PCB STREQUAL X7 AND PCBREV STREQUAL ZORRO)
  set(STORAGE_CONVERSIONS 221)
endif()

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
