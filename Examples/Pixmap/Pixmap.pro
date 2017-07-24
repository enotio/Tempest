TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11


include(../common.pri)

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib -l"Tempest"

SOURCES += main.cpp

