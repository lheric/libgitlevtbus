#-------------------------------------------------
#
# Project created by QtCreator 2013-08-01T14:53:53
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = examples
CONFIG   += console
CONFIG   += c++11
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH    += ../../libgitlevtbus/src

LIBS += -L$${OUT_PWD}/..

CONFIG(debug, debug|release){
    LIBS += -lGitlEvtBusd
}
CONFIG(release, debug|release){
    LIBS += -lGitlEvtBus
}
