include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(shared_NAMES
  genericpanel
  verticalscrollarea
  autobitsetcheckbox
  autocheckbox
  autocombobox
  autodoublespinbox
  autohexspinbox
  autolabel
  autolineedit
  autoprecisioncombobox
  autoslider
  autospinbox
  autotimeedit
  curveimagewidget
  curvedialog
)

set(shared_UIS
  curvedialog.ui
)

foreach(name ${shared_NAMES})
  set(shared_SRCS ${shared_SRCS} ${name}.cpp)
  set(shared_HDRS ${shared_HDRS} ${name}.h)
endforeach()

qt5_wrap_ui(shared_SRCS ${shared_UIS})
qt5_wrap_cpp(shared_SRCS ${shared_HDRS})

# add after moc generation to stop warnings
set(shared_SRCS ${shared_SRCS}
  autowidget.h
  autowidget.cpp
  curveimage.h
  curveimage.cpp
)

add_library(shared ${shared_SRCS})
target_link_libraries(shared PRIVATE datamodels Qt5::Widgets ${CPN_COMMON_LIB})
