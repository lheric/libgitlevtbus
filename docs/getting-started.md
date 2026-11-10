# Getting started

## Build the library

libgitlevtbus builds with qmake into a static library:

```sh
git clone https://github.com/lheric/libgitlevtbus.git
mkdir build && cd build
qmake ../libgitlevtbus/libgitlevtbus.pro -r "CONFIG+=release"
make            # nmake or jom with MSVC
```

This builds the library, the tests and an example. The library is placed in the build directory: `GitlEvtBus` for a release build, `GitlEvtBusd` for a debug build.

To use it in your own qmake project, add:

```text
QT       += core
CONFIG   += c++17
INCLUDEPATH += /path/to/libgitlevtbus/src
LIBS     += -L/path/to/build -lGitlEvtBus    # -lGitlEvtBusd for debug builds
```

## Your first module

A **module** is anything that listens to events. Create a `GitlModule`, subscribe to an event name, and dispatch an event with that name:

```cpp
#include "gitlmodule.h"
#include <QDebug>

int main()
{
    GitlModule cModule;

    cModule.subscribeToEvtByName("I am a test event", [](const GitlEvent&)
    {
        qDebug() << "Hello GitlEvtBus!";
        return true;
    });

    GitlEvent cEvent("I am a test event");
    cEvent.dispatch();          // prints "Hello GitlEvtBus!"
    return 0;
}
```

When the module and the sender are on the same thread, as here, the callback runs before `dispatch()` returns. See [Threading](threading.md) for other threads.

## Modules as classes

Usually a module is a class of your own that derives from `GitlModule`. Use `MAKE_CALLBACK` to subscribe a member function:

```cpp
class Logger : public GitlModule
{
public:
    Logger()
    {
        subscribeToEvtByName("UserLoggedIn", MAKE_CALLBACK(Logger::onLogin));
    }

    bool onLogin(const GitlEvent& rcEvt)
    {
        qDebug() << rcEvt.getParameter("user").toString() << "logged in";
        return true;
    }
};
```

From outside the class, use `MAKE_CALLBACK_OBJ(object, Class::method)` instead.

A module holds at most one callback per event name: subscribing again to the same name replaces the previous callback. Use `unsubscribeToEvtByName()` to stop listening. When a module is destroyed, it stops receiving events automatically.

## Carry data in parameters

Any event can carry named parameters, stored as `QVariant`:

```cpp
GitlEvent cEvt("UserLoggedIn");
cEvt.setParameter("user", "alice");
cEvt.setParameter("attempts", 3);
cEvt.dispatch();
```

Listeners read them with `getParameter()` and check for them with `hasParameter()`. For data with a fixed structure, a [custom event class](custom-events.md) is safer, because the compiler checks the field names and types.

## Event buses

Events travel on an **event bus**. By default, every module and every `dispatch()` use one global bus. For separate parts of a program, you can create separate buses, so their events don't mix:

```cpp
GitlEventBus* pcBus = GitlEventBus::create();   // you own it, and must delete it

GitlModule cModule(pcBus);                      // attached to pcBus
GitlEvent("Tick").dispatch(pcBus);              // only modules on pcBus receive it
```

- A module can move to another bus with `attach(pcOtherBus)`, keeping its subscriptions, or stop receiving events with `detach()`.
- A module can dispatch on its own bus with `cModule.dispatchEvt(cEvt)`.
- Deleting a bus is safe while modules are attached: they are disconnected, and their `getEventBus()` returns `nullptr`.
