Introduction
=============
[![Build Status](https://travis-ci.org/lheric/libgitlevtbus.png?branch=master)](https://travis-ci.org/lheric/libgitlevtbus)

libgitlevtbus is an open-source event bus (or message bus) based on Qt under BSD lisence.

Features
========
1. Easy to use (c++11 feature supported: lambda expression, member function callback, ...)
2. Custom event support (carry custom parameters)
3. Events can be deliverd across threads

Requirements
============
Qt 5.1.0 or later with MSVC, gcc or intel complier

**c++11 support required**

Quick start
===========
See examples/examples.pro
```c++
#include "gitlmodual.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    GitlModual cModual;

    /// subscribe to an event
    cModual.subscribeToEvtByName("I am a test event",
    [](GitlEvent& rcEvt)->bool
    {
        qDebug() << "Hello GitlEvtBus!";
        return true;
    }
    );

    GitlEvent cEvent("I am a test event");              ///< create an event
    cEvent.dispatch();                                  ///< dispatch
    /// output: "Hello GitlEvtBus!"
    return 0;
}
```


For more, you may refer to the code in test/testcase.cpp
