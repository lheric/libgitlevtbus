# Improvement Plan

Code review of libgitlevtbus, 2026-11-05, against commit `343e0a1`.

Items are in priority order. Check one off when its fix is merged. Line numbers refer to the reviewed commit.

---

## 1. Correctness bugs (high priority)

### 1.1 `SCOPE_EXIT` is broken — `src/gitldef.h:153-164`
- [x] `scope_exit_##__LINE__` does not expand `__LINE__`, because `##` stops macro expansion. Every use declares the same variable, `scope_exit___LINE__`, so two `SCOPE_EXIT`s in one scope won't compile. Fix it with a two-level concatenation helper:
  ```cpp
  #define GITL_CAT_(a, b) a##b
  #define GITL_CAT(a, b)  GITL_CAT_(a, b)
  #define SCOPE_EXIT(code) auto GITL_CAT(scope_exit_, __LINE__) = MakeScopeExit([=](){code;})
  ```
- [x] `ScopeExit` is copyable, and `MakeScopeExit` returns it by value. Before C++17 the compiler doesn't have to elide that copy, so the cleanup code can run twice. Delete the copy constructor and add a move constructor that disarms the source, or require C++17.

### 1.2 A missing `CLONABLE` silently slices events — `src/gitleventbus.cpp:63`, `src/gitlevent.h:39-47`
- [x] `post()` copies the event with `rcEvt.clone()`. If a subclass forgets `CLONABLE(...)`, the copy is a plain `GitlEvent`, and a listener's `static_cast<CustomEvent&>` is undefined behaviour. Add a guard in `post()`:
  ```cpp
  Q_ASSERT_X(typeid(*pcEvtCopy) == typeid(rcEvt), "GitlEventBus::post",
             "Event subclass is missing CLONABLE(...)");
  ```
- [x] Fix the doc comment in `gitlevent.h`. It tells users to add `VIRTUAL_COPY_PATTERN(...)`, but that macro doesn't exist; the real one is `CLONABLE(...)`.

### 1.3 The "cross-thread" feature isn't thread-safe
- [ ] `m_cListeningEvts` (`src/gitlmoduledelegate.h:98`) is a plain `QMap` with no lock. `subscribeToEvtByName` can write it on one thread while `detonate` reads it on another, which is a data race. Guard it with a `QMutex` or `QReadWriteLock`.
- [ ] One cloned event is shared by every subscriber through a `QSharedPointer`, and callbacks get a **non-const** `GitlEvent&`. Subscribers on different threads can therefore mutate the same event at the same time, and one can change what the others see. Change `GitlCallBack` to `std::function<bool(const GitlEvent&)>`.
- [ ] Destruction race: a module subclass destroys its own members before the base `GitlModule` destroys `m_cDelegate`. A callback that fires on another thread during that window reaches a half-destroyed object. Have `~GitlModule()` call `detach()` first, and document that subclasses used across threads should also detach in their own destructor.

### 1.4 Dangling bus pointer
- [ ] `GitlEventBus::create()` returns a raw pointer with no stated owner, and modules keep a plain `GitlEventBus*`. Deleting the bus leaves every attached module pointing at freed memory. Store a `QPointer<GitlEventBus>`, or have the bus detach its modules when it's destroyed. Document who owns a bus returned by `create()`.

### 1.5 Subscribing twice silently replaces the first callback — `src/gitlmoduledelegate.cpp:47`
- [ ] `QMap::insert` overwrites the existing callback. Either document "one handler per event per module" or switch to `QMultiMap` or a list.

---

## 2. API design (medium priority)

- [ ] **Callback return value is ignored.** `GitlCallBack` returns `bool`, but `detonate` throws it away. Either use it (for example, "handled, stop") or change the type to `void`.
- [ ] **Const-correctness:**
  - `GitlModule::dispatchEvt(GitlEvent&)` (`src/gitlmodule.h:73`) takes a non-const reference, so `dispatchEvt(GitlEvent("X"))` won't compile. It should take `const GitlEvent&`.
  - `GitlModule::getEventBus()` should be `const`.
  - `GitlEvent::hasParameter(QString)` and `GitlEventParam::hasParameter(QString)` copy their argument instead of taking a `const QString&`.
- [ ] **Delivery scales poorly.** Every event is sent to every module on the bus, and each module then looks up the event name. If the bus kept a `QHash<QString, QList<subscriber>>`, an event would only reach the modules that subscribed to it.
- [ ] **Raw-string event names** make typos into silent no-ops. Consider recommending `static const QString` constants, or adding a typed `subscribe<EventT>()` overload.
- [ ] **Noisy parameter lookup.** `getParameter` logs `qWarning` when a parameter is missing. Add an overload with a default value: `getParameter(name, defaultValue)`.
- [ ] **Dead code:** remove `xIsListenToEvt` and `m_pcDelegator`, which are never used. `setParameter` always returns `true`; return `void` instead.
- [ ] **Clean-ups:** use `nullptr` everywhere instead of mixing it with `NULL`. Remove `using namespace std;` and the unused `<iostream>` from `src/gitlmoduledelegate.cpp`.

---

## 3. Property and field macros, `src/gitldef.h` (medium priority)

- [ ] **Non-const getters bypass the setter.** The property getters return `type&`, so callers can skip the setter and the change signal never fires. `test/TestMacro/testmacro.cpp:62` does exactly that. Return `const type&` from the `ADD_QPROP_*` getters.
- [ ] **Setters emit even when nothing changed.** Add `if (m_x == v) return;` before emitting, as Qt's convention expects, and assign with `std::move`.
- [ ] **The change signal takes a non-const reference** (`void name##Changed(type& name)`). Use `const type&` or pass by value.
- [ ] **`VALUE_CLIP`** evaluates its arguments more than once. Replace it with `qBound` or `std::clamp`.
- [ ] **`CONCATE`** doesn't concatenate anything; it protects commas inside macro arguments. Rename it, for example to `GITL_ARGS`, and keep the old name as a deprecated alias.
- [ ] **Singleton:** the macro locks a mutex on every `getInstance()` call. A function-local static (`static T inst; return &inst;`) is thread-safe since C++11 and simpler.
- [ ] **Name collisions:** unprefixed macro names such as `ADD_FIELD` and `CLONABLE` can clash with user code. Consider a `GITL_` prefix.

---

## 4. Build and CI (medium priority)

- [ ] **Include paths depend on the folder name.** `INCLUDEPATH += ../../libgitlevtbus/src` in `examples/examples.pro:21` and `test/TestGitlEvtBus/TestGitlEvtBus.pro:18` only works if the repo is cloned into a folder named `libgitlevtbus`. Use `$$PWD/../src` or `$$PWD/../../src`.
- [ ] **`src/src.pro`:** remove `QT += widgets` (this is a core-only library), the duplicate `CONFIG += c++11`, and `CONFIG += console`, which means nothing for a static library.
- [ ] **Legacy settings:** `CONFIG += ordered` is deprecated. Replace it with `.depends`.
- [ ] **Signal/slot setup:**
  - Switch to the new-style `connect(this, &GitlEventBus::eventTriggered, pcModule, &GitlModuleDelegate::detonate)` to get compile-time checking.
  - Move `Q_DECLARE_METATYPE(QSharedPointer<GitlEvent>)` into the header.
  - Call `qRegisterMetaType` once instead of on every `registerModule`.
- [ ] **CI no longer runs.** Travis CI for open source is gone, and the config targets Ubuntu Trusty with an old Ubuntu SDK PPA. It also never ran the tests. Replace it with GitHub Actions: a Qt 5.15 + Qt 6 matrix on Linux, macOS and Windows that runs the test binaries.
- [ ] **Consider CMake** (and optionally `find_package` support), since Qt 6 has moved away from qmake.

---

## 5. Tests (medium priority)

- [ ] **`test_signals` checks nothing** (`test/TestMacro/testmacro.cpp:73-77`). It sets `bTriggered` but never asserts it, so the test passes even if the signal never fires. Add `QVERIFY(bTriggered)`.
- [ ] **Add a cross-thread delivery test**, using a module that lives on a `QThread` and `QTRY_VERIFY`. This is the README's main claim and nothing tests it.
- [ ] **Add the other missing cases:** `detach()`, deleting a bus or module while events are in flight, subscribing twice to one event, and a subclass that forgets `CLONABLE`.
- [ ] **Fix the leak:** `multiplyEventBus` never deletes `pcBus1` or `pcBus2` (`test/TestGitlEvtBus/testcase.cpp:191-192`).

---

## 6. Documentation (low priority)

- [ ] Fix the README typos: "lisence", "deliverd", "complier".
- [ ] The README points to `test/testcase.cpp`; the file is actually `test/TestGitlEvtBus/testcase.cpp`.
- [ ] Replace the dead Travis badge with a GitHub Actions badge.
- [ ] Replace "Qt 5.1.0 or later" with the Qt versions CI actually covers.
- [ ] Document the threading model: callbacks run on the thread where the module was created, and that thread needs a running event loop.

---

## Where to start

Do these first, because each is small and together they fix the real memory-safety and threading problems:

1. The `SCOPE_EXIT` fix (1.1)
2. The clone check (1.2)
3. The `const GitlEvent&` callbacks (1.3)
4. `QPointer` for the bus (1.4)
5. `detach()` in `~GitlModule()` (1.3)

After that, fix the include paths and set up GitHub Actions (section 4), so the fixes above are actually tested.
