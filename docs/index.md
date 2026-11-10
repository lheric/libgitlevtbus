# libgitlevtbus

libgitlevtbus is an event bus (message bus) for Qt applications. Parts of your program, called **modules**, subscribe to events by name or by type, and any code can dispatch events to them, from any thread, without the sender and the receivers knowing about each other.

```cpp
GitlModule cModule;
cModule.subscribeToEvtByName("Hello", [](const GitlEvent&)
{
    qDebug() << "Hello GitlEvtBus!";
    return true;
});

GitlEvent("Hello").dispatch();   // prints "Hello GitlEvtBus!"
```

## Features

- **Simple to use:** subscribe with a lambda or a member function, and dispatch with one call.
- **Typed events:** declare an event class with `GITL_EVENT`, and subscribe to it by type, with no name strings and no casts. Mistakes are caught at compile time.
- **Cross-thread delivery:** callbacks run on the thread the receiving module lives in. Dispatch synchronously (wait for every listener), asynchronously, or the default way in between.
- **Efficient:** an event only reaches the modules subscribed to it.
- **Several buses:** use the global bus, or create separate buses for separate parts of your program.

## Requirements

- **Qt 5.15 or Qt 6.** Only the Qt Core module is used.
- **A C++17 compiler**, such as GCC, Clang or MSVC.

CI builds and tests Qt 5.15 and Qt 6 on Linux (GCC), macOS (Clang) and Windows (MSVC).

## Where next

1. [Getting started](getting-started.md): build the library, write your first module, use parameters and event buses.
2. [Custom events](custom-events.md): send your own data with typed events.
3. [Sync and async dispatch](dispatch.md): choose when listeners run and whether the sender waits.
4. [Threading](threading.md): deliver events across threads.

libgitlevtbus is open source under the BSD license. The source code is on [GitHub](https://github.com/lheric/libgitlevtbus).
