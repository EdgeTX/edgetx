cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
find_package(Qt5Widgets)

project(maxLibQtWidgets)

set(SRCS
	"${CMAKE_CURRENT_LIST_DIR}/ActionPushButton.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ExportableTableView.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/OverlayStackLayout.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ScrollableMessageBox.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/TimerEdit.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/TreeComboBox.cpp"
)

set(HDRS
  "${CMAKE_CURRENT_LIST_DIR}/ActionPushButton.h"
  "${CMAKE_CURRENT_LIST_DIR}/BuddyLabel.h"
  "${CMAKE_CURRENT_LIST_DIR}/CollapsingToolBar.h"
	"${CMAKE_CURRENT_LIST_DIR}/ExportableTableView.h"
	"${CMAKE_CURRENT_LIST_DIR}/OverlayStackLayout.h"
	"${CMAKE_CURRENT_LIST_DIR}/RoundedMessageBox.h"
	"${CMAKE_CURRENT_LIST_DIR}/ScrollableMessageBox.h"
	"${CMAKE_CURRENT_LIST_DIR}/TimerEdit.h"
	"${CMAKE_CURRENT_LIST_DIR}/TreeComboBox.h"
)

add_library(${PROJECT_NAME} ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets)
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_USE_QSTRINGBUILDER)

# need to push this upstream
set(BUILT_LIBRARIES ${BUILT_LIBRARIES} ${PROJECT_NAME} PARENT_SCOPE)
