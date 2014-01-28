#-------------------------------------------------
#
# Project created by QtCreator 2014-01-06T23:26:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

android: {
  CONFIG += opengles
  QT += opengl
  }

TARGET = QTempest
TEMPLATE = app

INCLUDEPATH += ../../Tempest/include
LIBS += -L../../lib/ -lTempest

DESTDIR = ../bin

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    bind/qtempestwidget.cpp \
    renderwidget.cpp

HEADERS  += mainwindow.h \
    bind/qtempestwidget.h \
    renderwidget.h

FORMS    += mainwindow.ui \
    renderwidget.ui

OTHER_FILES += \
    ../bin/shader/basic.fs.glsl \
    ../bin/shader/basic.vs.glsl \
    android/AndroidManifest.xml

ANDROID_EXTRA_LIBS = ../../lib/libTempest.so

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
