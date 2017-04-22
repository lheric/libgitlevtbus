#-------------------------------------------------
#
# Project created by QtCreator 2013-05-28T02:26:05
#
#-------------------------------------------------

QT       += core testlib

QT       -= gui

TARGET = TestMacro
CONFIG   += console
CONFIG   += c++11
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH    += ../../../libgitlevtbus/src

SOURCES += \
    testmacro.cpp

LIBS += -L$${OUT_PWD}/..

