# Sync and async dispatch

An event can be dispatched three ways. Each is available on the event (`cEvt.dispatchSync()`), on a module (`cModule.dispatchSync(cEvt)`) and on a bus (`pcBus->postSync(cEvt)`):

| Call | Copies the event? | Needs `CLONABLE` / `GITL_EVENT`? | Listeners on this thread | Listeners on other threads | Returns |
|---|---|---|---|---|---|
| `dispatchSync` | No | No | Run immediately | Run on their threads, in parallel | After **every** listener has finished |
| `dispatchAsync` | Yes | Yes | Run later | Run later | Immediately |
| `dispatch` / `dispatchEvt` (default) | Yes | Yes | Run immediately | Run later | After this thread's listeners |

## dispatchSync: wait for every listener

```cpp
PlayerJoined cEvt("alice");    // a local variable
cModule.dispatchSync(cEvt);    // every listener, on every thread, has finished here
```

Listeners receive your object itself, not a copy, and the call waits until they are done with it. That's why a local variable works, and why the event class needs no `CLONABLE`. Listeners on other threads run in parallel, on their own threads.

If a receiving module is deleted before it handles the event, it's skipped, and the call still returns.

!!! warning "dispatchSync can hang"
    - It blocks forever if a receiving thread has no running Qt event loop.
    - It deadlocks if two threads `dispatchSync` to each other at the same time, because each waits for the other.

    If you don't need to wait, prefer `dispatch` or `dispatchAsync`.

The bus's debugging signal `eventTriggered` isn't emitted for `dispatchSync`, because no copy exists that could safely outlive the call.

## dispatchAsync: never run a listener before returning

```cpp
cModule.dispatchAsync(cEvt);   // returns at once; listeners run later, even on this thread
```

The event is copied when you dispatch, so you can change or destroy your object right away. Every listener, including those on your own thread, runs later from its thread's event loop. That's useful to avoid nested dispatches, where a callback sends an event that's handled before the callback finishes.

The calling thread needs a running event loop for its own listeners.

## dispatch: the default

`cEvt.dispatch()` and `cModule.dispatchEvt(cEvt)` copy the event. Listeners on your thread run immediately, before the call returns; listeners on other threads run later, on their own threads.

## Which one to use

- **Use `dispatch` by default.** It never blocks, and same-thread listeners have finished when it returns.
- **Use `dispatchSync`** when you need every listener, on every thread, to have finished before you continue, or to send an event class without `CLONABLE`.
- **Use `dispatchAsync`** when listeners must not run inside your current call stack.
