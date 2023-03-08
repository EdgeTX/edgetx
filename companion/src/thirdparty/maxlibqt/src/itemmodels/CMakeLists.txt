
project(maxLibQtItemModels)

set(SRCS
	"${CMAKE_CURRENT_LIST_DIR}/GroupedItemsProxyModel.cpp"
)

set(HDRS
	"${CMAKE_CURRENT_LIST_DIR}/GroupedItemsProxyModel.h"
)

qt5_wrap_cpp(SRCS ${HDRS})

add_library(${PROJECT_NAME} ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_USE_QSTRINGBUILDER)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# need to push this upstream
set(BUILT_LIBRARIES ${BUILT_LIBRARIES} ${PROJECT_NAME} PARENT_SCOPE)
