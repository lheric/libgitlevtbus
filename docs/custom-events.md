# Custom events

To send your own data, derive from `GitlEvent` and add `GITL_EVENT(YourEvent)` as the first line of the class:

```cpp
class PlayerJoined : public GitlEvent
{
    GITL_EVENT(PlayerJoined)
public:
    explicit PlayerJoined(const QString& strName = QString()) { m_strName = strName; }
    ADD_CLASS_FIELD(QString, strName, getName, setName)
};
```

`ADD_CLASS_FIELD` declares a member `m_strName` with a getter and a setter. Any ordinary members and methods work too.

## Receive typed events

Subscribe by type with `subscribeToEvt<T>()`. There's no event-name string, and the listener receives the event as its real type, without a cast:

```cpp
cModule.subscribeToEvt<PlayerJoined>([](const PlayerJoined& rcEvt)
{
    qDebug() << rcEvt.getName() << "joined";
});
```

Use `unsubscribeToEvt<PlayerJoined>()` to stop listening. Listeners don't need to return anything.

## Send them

Any [dispatch style](dispatch.md) works:

```cpp
PlayerJoined cEvt("alice");
cModule.dispatchSync(cEvt);     // or cEvt.dispatch(), cModule.dispatchAsync(cEvt), ...
```

## How typed events behave

- **The event is named after its class.** `PlayerJoined::staticEvtName()` is `"PlayerJoined"`, and every instance has that name from construction, so there's no name string to mistype, and constructors don't need to pass one.
- **Mistakes are caught at compile time.** Using a class that doesn't declare its own `GITL_EVENT` with the typed API is a compile error:

    ```text
    error: static assertion failed: EvtT must declare GITL_EVENT(EvtT) in its own class body
    ```

    That includes a subclass that forgets it, which would otherwise silently use its parent's name.

- **A subclass with its own `GITL_EVENT` is a separate event.** If `VipJoined` derives from `PlayerJoined` and declares `GITL_EVENT(VipJoined)`, subscribers to `PlayerJoined` don't receive it.
- **Wrong types are skipped.** An event that has the name `"PlayerJoined"` but isn't a `PlayerJoined`, such as a plain `GitlEvent("PlayerJoined")`, is skipped with a warning instead of being mis-cast.
- **Typed and string subscriptions share one slot per name.** `subscribeToEvt<PlayerJoined>()` replaces a `subscribeToEvtByName("PlayerJoined", ...)` on the same module, and the other way round.

## Older style: CLONABLE

Before typed events, custom events were declared with `CLONABLE` and received by name, with a manual cast. This still works:

```cpp
class PlayerJoined : public GitlEvent
{
    CLONABLE(PlayerJoined)
public:
    PlayerJoined() : GitlEvent("PlayerJoined") {}
    ADD_CLASS_FIELD(QString, strName, getName, setName)
};

cModule.subscribeToEvtByName("PlayerJoined", [](const GitlEvent& rcEvt)
{
    const auto& rcJoined = static_cast<const PlayerJoined&>(rcEvt);
    qDebug() << rcJoined.getName();
    return true;
});
```

`CLONABLE` is needed because `dispatch()`, `dispatchEvt()` and `dispatchAsync()` copy the event, and copying through a `GitlEvent&` requires a virtual `clone()`. If it's missing, the event is dropped with an error at run time. `GITL_EVENT` includes `clone()`, and `dispatchSync()` doesn't copy, so it needs neither.

Plain `GitlEvent`s carrying their data in [parameters](getting-started.md#carry-data-in-parameters) need neither macro.
