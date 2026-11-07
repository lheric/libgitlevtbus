Introduction
=============
[![CI](https://github.com/lheric/libgitlevtbus/actions/workflows/ci.yml/badge.svg)](https://github.com/lheric/libgitlevtbus/actions/workflows/ci.yml)

libgitlevtbus is an open-source event bus (or message bus) based on Qt under BSD lisence.

Features
========
1. Easy to use (modern C++: lambda expressions, member function callbacks, ...)
2. Custom event support (carry custom parameters)
3. Events can be deliverd across threads

Requirements
============
- **Qt 5.15 or Qt 6.** Only the Qt Core module is used.
- **A C++17 compiler**, such as GCC, Clang or MSVC.

CI builds and tests Qt 5.15 and Qt 6.8 on Linux (GCC), macOS (Clang) and Windows (MSVC).

Quick start
===========
See examples/examples.pro
```c++
#include "gitlmodule.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    GitlModule cModule;

    /// subscribe to an event
    cModule.subscribeToEvtByName("I am a test event",
    [](const GitlEvent& rcEvt)->bool
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
