TARGET = maxLibQtItemModels
TEMPLATE = lib

QT += core

DESTDIR = $${OUT_PWD}/bin
DEFINES += QT_USE_QSTRINGBUILDER

include(maxLibQtItemModels.pri)

OTHER_FILES += $$PWD/CMakeLists.txt
