QT += core gui widgets

TEMPLATE = app
CONFIG += c++17
CONFIG += debug
CONFIG += qt
CONFIG += object_parallel_to_source

SOURCES += $$files($$PWD/*.cpp, true)
HEADERS += $$files($$PWD/*.h, true)

INCLUDEPATH += $$PWD

DESTDIR     = ../bin
MOC_DIR     = ../build/moc
OBJECTS_DIR = ../build/obj
RCC_DIR     = ../build/rcc
