TEMPLATE = app
CONFIG -= qt
CONFIG -= app_bundle
CONFIG += c++17

include(C:/Users/bruger/Documents/simon/prog/vendor/pugixml.pri)

SOURCES += src/maze.cpp

HEADERS += \
    src/wikipedia_maze.h \
    simple_maze.h

DISTFILES += \
    .gitignore
