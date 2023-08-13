include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(updates_NAMES
  updates
  updatesdialog
  updateoptionsdialog
  updatestatus
  updatenetwork
  repodatamodels
  repometadata
  reporeleases
  repoassets
  repo
  updateparameters
  updateinterface
  updatefirmware
  updatecompanion
  updatesdcard
  updatesounds
  updatethemes
  updatemultiprotocol
  updatefactories
  chooserdialog
)

foreach(name ${updates_NAMES})
  set(updates_SRCS ${updates_SRCS} ${name}.cpp)
  set(updates_HDRS ${updates_HDRS} ${name}.h)
endforeach()

set(updates_UIS
  updatesdialog.ui
  updateoptionsdialog.ui
  chooserdialog.ui
)

set(updates_QTS
  updates.h
  updatesdialog.h
  updateoptionsdialog.h
  updatestatus.h
  updatenetwork.h
  repodatamodels.h
  reporeleases.h
  repoassets.h
  repo.h
  updateparameters.h
  updateinterface.h
  updatefactories.h
  chooserdialog.h
)

qt5_wrap_ui(updates_SRCS ${updates_UIS})
qt5_wrap_cpp(updates_SRCS ${updates_QTS})

add_library(updates ${updates_SRCS} ${updates_HDRS})
target_link_libraries(updates PRIVATE ${CPN_COMMON_LIB})
