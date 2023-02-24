TARGET = maxLibQtQuickControls
TEMPLATE = aux

include(controls.pri)

QT += quick qml

OTHER_FILES += qmldir
OTHER_FILES += $$QML_SRC
OTHER_FILES += tests.qml
