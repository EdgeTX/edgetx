set(qcustomplot_SRCS
  qcustomplot.cpp
)

set(qcustomplot_HDRS
  qcustomplot.h
)

qt_wrap_cpp(qcustomplot_SRCS ${qcustomplot_HDRS})

add_library(qcustomplot ${qcustomplot_SRCS})
target_link_libraries(qcustomplot PRIVATE ${CPN_COMMON_LIB} Qt::Widgets Qt::PrintSupport)
