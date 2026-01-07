QT += core
TEMPLATE = app
CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += $$files($$PWD/*.cpp)
HEADERS += $$files($$PWD/*.h)

INCLUDEPATH += $$PWD/../src
INCLUDEPATH += $$PWD/../src/engine

# Pull engine sources directly into the test binary.
SOURCES += $$files($$PWD/../src/engine/*.cpp)
HEADERS += $$files($$PWD/../src/engine/*.h)

DESTDIR = ../bin
OBJECTS_DIR = ../build/tests-obj
MOC_DIR = ../build/tests-moc
