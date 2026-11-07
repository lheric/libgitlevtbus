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

CI builds and tests Qt 5.15 and Qt 6 on Linux (GCC), macOS (Clang) and Windows (MSVC). It uses Qt 6.8 LTS on Linux and Windows, and Qt 6.11 on macOS.

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

Threading
=========
Events can be dispatched from any thread. Where a callback runs depends on the receiving module, not on the sender:

- **Callbacks run on the thread the module lives in.** That's the thread that created the module, unless you move it with `moveModuleToThread()`.
- **Same thread: delivered immediately.** If the event is dispatched on the module's own thread, the callback runs before `dispatch()` returns.
- **Other thread: queued.** If the event is dispatched from a different thread, it's queued, and `dispatch()` returns before the callback runs.
- **The receiving thread needs a running Qt event loop.** Queued events are delivered by that loop. A `QThread` runs one by default, but a module created on a `std::thread` or a thread without `exec()` silently never receives events from other threads.
- **Callbacks receive `const GitlEvent&`.** Listeners on different threads share one copy of each event, so it must not be modified.
- **Subscribing, unsubscribing, attaching and detaching are thread-safe.**

If your module class is also a `QObject`, then `moveToThread()` moves only the `QObject` part. Call `moveModuleToThread()` as well, so the callbacks move too:

```c++
class Worker : public QObject, public GitlModule { /* ... */ };

Worker* pcWorker = new Worker;
pcWorker->moveToThread(&cThread);         // QObject part: slots, timers
pcWorker->moveModuleToThread(&cThread);   // GitlModule part: event callbacks
```

Like `moveToThread()`, it must be called from the thread the module currently lives in. `moduleThread()` returns the thread the callbacks run on.

For more, see the tests in `test/TestGitlEvtBus/testcase.cpp`. `crossThreadDelivery`, `moveModuleToThread` and `qobjectSubclass` cover threading.
