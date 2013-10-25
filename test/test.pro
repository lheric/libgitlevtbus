#-------------------------------------------------
#
# Project created by QtCreator 2013-05-28T02:26:05
#
#-------------------------------------------------

QT       += core testlib

QT       -= gui

TARGET = TestGitlEvtBus
CONFIG   += console
CONFIG   += c++11
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH    += ../../libgitlevtbus/src

SOURCES += \
    testcase.cpp

LIBS += -L$${OUT_PWD}/..

CONFIG(debug, debug|release){
    LIBS += -lGitlEvtBusd
}
CONFIG(release, debug|release){
    LIBS += -lGitlEvtBus
}
