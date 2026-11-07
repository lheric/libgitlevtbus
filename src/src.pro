#-------------------------------------------------
#
# Project created by QtCreator 2012-11-02T17:27:25
#
#-------------------------------------------------

QT       += core
QT       -= gui

CONFIG   += c++17
CONFIG(debug, debug|release){
    TARGET = GitlEvtBusd
}
CONFIG(release, debug|release){
    TARGET = GitlEvtBus
}

DESTDIR = $${OUT_PWD}/..

CONFIG += console
CONFIG += staticlib
CONFIG -= app_bundle

TEMPLATE = lib

SOURCES += \
    gitlevent.cpp \
    gitleventbus.cpp \
    gitlmoduledelegate.cpp \
    gitlmodule.cpp \
    gitleventparam.cpp

HEADERS += \
    gitldef.h \
    gitlevent.h \
    gitleventbus.h \
    gitlmodule.h \
    gitlmoduledelegate.h \
    gitleventparam.h
