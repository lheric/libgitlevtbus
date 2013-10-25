Introduction
=============
[![Build Status](https://travis-ci.org/lheric/libgitlevtbus.png?branch=master)](https://travis-ci.org/lheric/libgitlmvc)

libgitlevtbus is an open-sourced event bus (or message bus) based on C++ Qt under BSD lisence.

Features
========
1. Easy to use
2. Custom event support
3. Events can be deliverd across threads

Complie
=======
Qt 5.0.1 or later with MSVC, gcc or intel complier

Quick start
===========
See examples/examples.pro
```
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
    /// output: "Hello GitlEvtBus!"*/
    return 0;
}
```


For more, you may refer to the code in test/testcase.cpp
