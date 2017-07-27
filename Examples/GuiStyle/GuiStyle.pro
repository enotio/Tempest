TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

include(../common.pri)

SOURCES += main.cpp \
    mainwindow.cpp \
    customstyle.cpp

HEADERS += \
    mainwindow.h \
    customstyle.h
