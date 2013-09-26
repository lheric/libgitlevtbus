#-------------------------------------------------
#
# Project created by QtCreator 2012-11-02T17:27:25
#
#-------------------------------------------------

QT       += core
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release){
    TARGET = GitlEvtBusd
}
CONFIG(release, debug|release){
    TARGET = GitlEvtBus
}

DESTDIR = $${OUT_PWD}/..

# c++11 enalbed
CONFIG += c++11
CONFIG += console
CONFIG += staticlib
CONFIG -= app_bundle

TEMPLATE = lib

SOURCES += \
    gitlevent.cpp \
    gitleventbus.cpp \
    gitlmodualdelegate.cpp \
    gitlmodual.cpp \
    gitleventparam.cpp

HEADERS += \
    gitldef.h \
    gitlevent.h \
    gitleventbus.h \
    gitlmodual.h \
    gitlmodualdelegate.h \
    gitleventparam.h
