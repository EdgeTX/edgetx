cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0020 NEW)

find_package(Qt5Core)
find_package(Qt5Widgets)

project(maxLibQtWidgets)

set(SRCS
	"${CMAKE_CURRENT_LIST_DIR}/ExportableTableView.cpp"
#	"${CMAKE_CURRENT_LIST_DIR}/ScrollableMessageBox.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/TimerEdit.cpp"
#	"${CMAKE_CURRENT_LIST_DIR}/TreeComboBox.cpp"
)

set(HDRS
	"${CMAKE_CURRENT_LIST_DIR}/ExportableTableView.h"
#	"${CMAKE_CURRENT_LIST_DIR}/ScrollableMessageBox.h"
	"${CMAKE_CURRENT_LIST_DIR}/TimerEdit.h"
#	"${CMAKE_CURRENT_LIST_DIR}/TreeComboBox.h"
)

qt5_wrap_cpp(SRCS ${HDRS})

add_library(${PROJECT_NAME} ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets)
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_USE_QSTRINGBUILDER)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# need to push this upstream
set(BUILT_LIBRARIES ${BUILT_LIBRARIES} ${PROJECT_NAME} PARENT_SCOPE)
