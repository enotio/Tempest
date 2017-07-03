include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt

DESTDIR = ../bin

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib/ -lTempest

HEADERS +=    

SOURCES += \
    main.cpp \
    utils_test.cpp \
    layout_test.cpp
