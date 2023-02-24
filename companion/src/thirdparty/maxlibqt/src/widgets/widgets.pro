TARGET = maxLibQtWidgets
TEMPLATE = lib

QT += core widgets

DESTDIR = $${OUT_PWD}/bin
DEFINES += QT_USE_QSTRINGBUILDER

include(maxLibQtWidgets.pri)

OTHER_FILES += $$PWD/CMakeLists.txt
