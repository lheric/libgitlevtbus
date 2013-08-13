Introduction
=============
https://travis-ci.org/lheric/libgitlevtbus.png?branch=master

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
#include "gitleventbus.h"
#include <QDebug>

/*!
 * This is a test modual
 */
class TestModual : public GitlModual
{
public:
    /// re-implement this method to do anything you want...
    virtual bool detonate( GitlEvent& rcEvt)
    {
        qDebug() << "I got notified!";
        return true;
    }
};


int main(int argc, char *argv[])
{
    TestModual cModual;
    cModual.subscribeToEvtByName("I am a test event");  ///< subscribe to an event

    GitlEvent cEvent("I am a test event");              ///< create an event
    cEvent.dispatch();                                  ///< dispatch
    ///< output: "I got notified!"
    
    return 0;
}
```


For more, you may refer to the code in test/testcase.cpp
