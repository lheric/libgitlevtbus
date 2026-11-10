# Threading

Events can be dispatched from any thread. Where a callback runs depends on the receiving module, not on the sender.

## The rules

- **Callbacks run on the thread the module lives in.** That's the thread that created the module, unless you move it with `moveModuleToThread()`.
- **Same thread: delivered immediately.** If the event is dispatched on the module's own thread, the callback runs before `dispatch()` returns.
- **Other thread: queued.** If the event is dispatched from a different thread, it's queued, and `dispatch()` returns before the callback runs. To wait for it, or to queue even on the same thread, see [Sync and async dispatch](dispatch.md).
- **The receiving thread needs a running Qt event loop.** Queued events are delivered by that loop. A `QThread` runs one by default, but a module created on a `std::thread`, or on a thread that never calls `exec()`, silently never receives events from other threads.
- **Callbacks receive `const GitlEvent&`.** Listeners on different threads may share one event object, so it must not be modified.
- **Subscribing, unsubscribing, attaching and detaching are thread-safe.**
- **An event only reaches modules that are subscribed when it's dispatched.** Modules that aren't subscribed to an event never see it, so they cost nothing. A module that unsubscribes before a queued event is delivered won't receive it.

## A module on a worker thread

Create the module, subscribe, then move it to the worker thread:

```cpp
QThread cThread;
cThread.start();                       // runs an event loop by default

GitlModule cWorker;
cWorker.subscribeToEvtByName("Job", [](const GitlEvent&)
{
    // runs on cThread
    return true;
});
cWorker.moveModuleToThread(&cThread);

GitlEvent("Job").dispatch();           // queued to cThread; returns at once
```

Like `QObject::moveToThread()`, `moveModuleToThread()` must be called from the thread the module currently lives in. `moduleThread()` returns the thread the callbacks run on.

Several modules can live on several threads. A single dispatch reaches every subscribed module, each on its own thread.

## Classes that are also a QObject

`GitlModule` is deliberately not a `QObject`, so a class can derive from both, for example a widget that is also a module. In such a class, `moveToThread()` moves only the `QObject` part. Call `moveModuleToThread()` as well, so the callbacks move too:

```cpp
class Worker : public QObject, public GitlModule { /* ... */ };

Worker* pcWorker = new Worker;
pcWorker->moveToThread(&cThread);         // QObject part: slots, timers
pcWorker->moveModuleToThread(&cThread);   // GitlModule part: event callbacks
```

## Destroying modules

A module stops receiving events as soon as it's destroyed or detached, including events that were already queued for it. Destroy a module on the thread it lives in, or after that thread has stopped.

If a subclass's callbacks use the subclass's own members, and the module may be destroyed from another thread, call `detach()` at the start of the subclass's destructor, so no callback runs while those members are being destroyed.
