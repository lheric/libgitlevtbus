#-------------------------------------------------
#
# Project created by QtCreator 2013-08-01T14:53:53
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = examples
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH    += ../../libgitlevtbus/src

LIBS += -L$${OUT_PWD}/..

Debug:   LIBS += -lGitlEvtBusd
Release: LIBS += -lGitlEvtBus
