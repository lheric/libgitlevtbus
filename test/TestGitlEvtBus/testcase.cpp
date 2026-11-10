/*******************************************************************************************
 * Copyright (c) 2013, Huang Li <lihuang55555@gmail.com>, IIPL <gitl.sysu.edu.cn>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, this list
 *   of conditions and the following disclaimer in the documentation and/or other
 *   materials provided with the distribution.
 * * Neither the name of the IIPL nor the names of its contributors may be used
 *   to endorse or promote products derived from this software without specific prior
 *   written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************************/
#include <QCoreApplication>
#include <iostream>
#include <QtTest/QtTest>
#include <QTest>
#include <QSharedPointer>
#include <QString>
#include <functional>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <QThread>
#include <QRegularExpression>
#include <QSemaphore>
#include "gitldef.h"
#include "gitlmodule.h"
#include "gitleventbus.h"
using namespace std;

/// test event bus
class TestModule : public GitlModule
{
public:
    TestModule(GitlEventBus* pcEventBus = NULL):
        GitlModule(pcEventBus)
    {
        this->m_bNotified = false;

    }

    void subscribeInsideClass()
    {
        subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK(TestModule::callback));
    }

    bool callback( const GitlEvent& rcEvt)
    {
        Q_UNUSED(rcEvt)
        this->m_bNotified = true;
        return true;
    }




    ADD_CLASS_FIELD(bool, bNotified, getNotified, setNotified)
};


/// custom event
class CustomEvent : public GitlEvent
{
    CLONABLE(CustomEvent)
public:
    CustomEvent( const QString& strEvtName ) : GitlEvent(strEvtName) { m_strCustomVar = "Custom String"; }
    ADD_CLASS_FIELD(QString, strCustomVar, getCustomVar, setCustomVar)
};

/// no CLONABLE: can only be sent with dispatchSync, which does not copy
class NoCloneEvent : public GitlEvent
{
public:
    explicit NoCloneEvent(int iValue) : GitlEvent("NO_CLONE_EVENT"), m_iValue(iValue) {}
    int m_iValue;
};

class CustomEventListener : public GitlModule
{
public:
    CustomEventListener()
    {
        this->m_bNotified = false;
    }

    bool callback( const GitlEvent& rcEvt)
    {
        const CustomEvent& rcCusEvt = static_cast<const CustomEvent&>(rcEvt);
        this->m_bNotified = true;
        this->m_strCustomVar = rcCusEvt.getCustomVar();
        return true;
    }



    ADD_CLASS_FIELD(bool, bNotified, getNotified, setNotified)
    ADD_CLASS_FIELD(QString, strCustomVar, getCustomVar, setCustomVar)
};



/// test case
class TestCase : public QObject
{
    Q_OBJECT

private slots:
    void lamdaListening()
    {
        TestModule cModule;
        cModule.subscribeToEvtByName("TEST_EVENT_1",
         [&](const GitlEvent& e)->bool
         {
            Q_UNUSED(e)
            cModule.setNotified(true);
            return true;
         });
        QVERIFY(!cModule.getNotified());
        GitlEvent cEvt("TEST_EVENT_1");
        cModule.dispatchEvt(cEvt);
        QVERIFY(cModule.getNotified());
    }


    void listenInsideClass()
    {
        TestModule cModule;
        cModule.subscribeInsideClass();
        QVERIFY(!cModule.getNotified());
        GitlEvent cEvt("TEST_EVENT_1");
        cEvt.dispatch();
        QVERIFY(cModule.getNotified());
    }

    void listenOutsideClass()
    {
        TestModule cModule;
        cModule.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule, TestModule::callback));
        QVERIFY(!cModule.getNotified());
        GitlEvent cEvt("TEST_EVENT_1");
        cModule.dispatchEvt(cEvt);
        QVERIFY(cModule.getNotified());
    }

    void unsubscribe()
    {
        TestModule cModule;
        cModule.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule, TestModule::callback));
        cModule.unsubscribeToEvtByName("TEST_EVENT_1");
        QVERIFY(!cModule.getNotified());
        GitlEvent cEvt("TEST_EVENT_1");
        cModule.dispatchEvt(cEvt);
        QVERIFY(!cModule.getNotified());
    }



    void oneToMany()
    {
        TestModule cSender;
        TestModule cModule1;
        TestModule cModule2;
        TestModule cModule3;
        cModule1.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule1, TestModule::callback));
        cModule2.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule2, TestModule::callback));
        cModule3.subscribeToEvtByName("TEST_EVENT_2", MAKE_CALLBACK_OBJ(cModule3, TestModule::callback));

        GitlEvent cEvt1("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt1);
        QVERIFY(cModule1.getNotified());
        QVERIFY(cModule2.getNotified());
        QVERIFY(!cModule3.getNotified());

        GitlEvent cEvt2("TEST_EVENT_2");
        cSender.dispatchEvt(cEvt2);
        QVERIFY(cModule3.getNotified());

    }

    void customEventTest()
    {
        CustomEventListener cModule;
        cModule.subscribeToEvtByName("TEST_EVENT_1",MAKE_CALLBACK_OBJ(cModule, CustomEventListener::callback));
        CustomEvent cEvt("TEST_EVENT_1");
        cEvt.dispatch();
        QVERIFY(cModule.getNotified());
        QVERIFY(cModule.getCustomVar() == QString("Custom String"));
    }

    void multiplyEventBus()
    {
        /// the guards are declared before the modules, so the modules are destroyed first
        GitlEventBus* pcBus1 = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus1);
        GitlEventBus* pcBus2 = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus2);

        TestModule cModule1(pcBus1); TestModule cModule2(pcBus1);
        TestModule cModule3(pcBus2); TestModule cModule4(pcBus2);

        /// all module are listening to the same events, but on different event buses.
        cModule1.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule1, TestModule::callback));
        cModule2.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule2, TestModule::callback));
        cModule3.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule3, TestModule::callback));
        cModule4.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule4, TestModule::callback));

        /// event
        CustomEvent cEvt("TEST_EVENT_1");

        /// no one get notified because no module is attached to the default event bus
        cEvt.dispatch();
        QVERIFY(!cModule1.getNotified());
        QVERIFY(!cModule2.getNotified());
        QVERIFY(!cModule3.getNotified());
        QVERIFY(!cModule4.getNotified());

        /// this will only notify module 1 & 2
        cEvt.dispatch(pcBus1);
        QVERIFY(cModule1.getNotified());
        QVERIFY(cModule2.getNotified());
        QVERIFY(!cModule3.getNotified());
        QVERIFY(!cModule4.getNotified());

        /// this will notify module 3 & 4
        cEvt.dispatch(cModule3.getEventBus());
        QVERIFY(cModule3.getNotified());
        QVERIFY(cModule4.getNotified());

        /// make sure everyone is attached to the correct event bus
        QVERIFY(cModule1.getEventBus() == pcBus1);
        QVERIFY(cModule2.getEventBus() == pcBus1);
        QVERIFY(cModule3.getEventBus() == pcBus2);
        QVERIFY(cModule4.getEventBus() == pcBus2);

        ///
        TestModule cModule5(pcBus1);
        cModule5.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule5, TestModule::callback));

        cEvt.dispatch(pcBus2);
        QVERIFY(!cModule5.getNotified());

        cModule5.attach(pcBus2);
        cEvt.dispatch(pcBus2);
        QVERIFY(cModule5.getNotified());

    }

    /// 1.4: deleting a bus must not leave modules with a dangling pointer
    void deletedBusIsSafe()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        TestModule cModule(pcBus);
        cModule.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule, TestModule::callback));
        QVERIFY(cModule.getEventBus() == pcBus);

        delete pcBus;
        QVERIFY(cModule.getEventBus() == nullptr);

        cModule.dispatchEvt(GitlEvent("TEST_EVENT_1"));    ///< must not crash
        QVERIFY(!cModule.getNotified());

        cModule.detach();                                  ///< must not crash either
    }

    /// a detached module receives nothing; re-attaching keeps its subscriptions
    void detachAndReattach()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);
        TestModule cModule(pcBus);
        cModule.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule, TestModule::callback));

        cModule.detach();
        QVERIFY(cModule.getEventBus() == nullptr);
        GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        QVERIFY(!cModule.getNotified());

        cModule.attach(pcBus);
        GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        QVERIFY(cModule.getNotified());
    }

    /// 1.3: a destroyed module is disconnected and its callbacks are not called
    void destroyedModuleIsDisconnected()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);
        int iCalls = 0;
        {
            GitlModule cModule(pcBus);
            cModule.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ ++iCalls; return true; });
            GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        }
        GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        QCOMPARE(iCalls, 1);
    }

    /// 1.5: subscribing twice to the same event replaces the first callback
    void subscribeTwiceReplaces()
    {
        TestModule cModule;
        int iFirst = 0, iSecond = 0;
        cModule.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ ++iFirst;  return true; });
        cModule.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ ++iSecond; return true; });
        cModule.dispatchEvt(GitlEvent("TEST_EVENT_1"));
        QCOMPARE(iFirst, 0);
        QCOMPARE(iSecond, 1);
    }

    /// a callback may subscribe/dispatch from inside a callback without deadlocking
    void reentrantCallback()
    {
        TestModule cModule;
        int iCalls = 0;
        cModule.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&)
        {
            ++iCalls;
            cModule.subscribeToEvtByName("TEST_EVENT_2", [&](const GitlEvent&){ ++iCalls; return true; });
            cModule.dispatchEvt(GitlEvent("TEST_EVENT_2"));
            return true;
        });
        cModule.dispatchEvt(GitlEvent("TEST_EVENT_1"));
        QCOMPARE(iCalls, 2);
    }

    /// 1.3: callbacks of a module created in a worker thread run in that thread
    void crossThreadDelivery()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        QThread cThread;
        QObject cContext;
        cContext.moveToThread(&cThread);
        cThread.start();

        std::atomic<int> iCalls(0);
        std::atomic<QThread*> pcCallbackThread(nullptr);
        GitlModule* pcModule = nullptr;

        /// create the module in the worker thread so its delegate lives there
        QMetaObject::invokeMethod(&cContext, [&]()
        {
            pcModule = new GitlModule(pcBus);
            pcModule->subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent& rcEvt)
            {
                Q_UNUSED(rcEvt)
                pcCallbackThread = QThread::currentThread();
                ++iCalls;
                return true;
            });
        }, Qt::BlockingQueuedConnection);

        GitlEvent("TEST_EVENT_1").dispatch(pcBus);   ///< posted from the main thread
        QTRY_COMPARE(iCalls.load(), 1);
        QVERIFY(pcCallbackThread.load() == &cThread);

        QMetaObject::invokeMethod(&cContext, [&](){ delete pcModule; }, Qt::BlockingQueuedConnection);
        cThread.quit();
        cThread.wait();
    }

    /// moveModuleToThread() makes a module created on the main thread run its callbacks on a worker
    void moveModuleToThread()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        QThread cThread;
        QObject cContext;
        cContext.moveToThread(&cThread);
        cThread.start();

        std::atomic<int> iCalls(0);
        std::atomic<QThread*> pcCallbackThread(nullptr);

        GitlModule cModule(pcBus);
        cModule.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&)
        {
            pcCallbackThread = QThread::currentThread();
            ++iCalls;
            return true;
        });
        QVERIFY(cModule.moduleThread() == QThread::currentThread());

        cModule.moveModuleToThread(&cThread);
        QVERIFY(cModule.moduleThread() == &cThread);

        GitlEvent("TEST_EVENT_1").dispatch(pcBus);   ///< posted from the main thread
        QTRY_COMPARE(iCalls.load(), 1);
        QVERIFY(pcCallbackThread.load() == &cThread);

        /// move it back from the thread it now lives in, before that thread stops
        QThread* pcMainThread = QThread::currentThread();
        QMetaObject::invokeMethod(&cContext, [&](){ cModule.moveModuleToThread(pcMainThread); },
                                  Qt::BlockingQueuedConnection);
        QVERIFY(cModule.moduleThread() == pcMainThread);
        cThread.quit();
        cThread.wait();

        GitlEvent("TEST_EVENT_1").dispatch(pcBus);   ///< same thread again: delivered directly
        QCOMPARE(iCalls.load(), 2);
        QVERIFY(pcCallbackThread.load() == pcMainThread);
    }

    /// deleting a module while events for it are still queued drops them safely
    void deletedModuleDropsQueuedEvents()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        QThread cThread;
        QObject cContext;
        cContext.moveToThread(&cThread);
        cThread.start();
        auto cStop = MakeScopeExit([&](){ cThread.quit(); cThread.wait(); });

        std::atomic<int> iDeletedCalls(0), iKeptCalls(0);
        GitlModule* pcDeleted = new GitlModule(pcBus);
        GitlModule* pcKept    = new GitlModule(pcBus);   ///< control: proves the events were queued
        pcDeleted->subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ ++iDeletedCalls; return true; });
        pcKept   ->subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ ++iKeptCalls;    return true; });
        pcDeleted->moveModuleToThread(&cThread);
        pcKept   ->moveModuleToThread(&cThread);

        /// park the worker until the events below are queued, then delete one module
        /// on its own thread while its events are still waiting in the queue
        QSemaphore cPosted;
        QMetaObject::invokeMethod(&cContext, [&]()
        {
            cPosted.acquire();
            delete pcDeleted;
        }, Qt::QueuedConnection);

        for(int i = 0; i < 3; ++i)
            GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        QCOMPARE(iKeptCalls.load(), 0);              ///< still queued
        cPosted.release();

        QMetaObject::invokeMethod(&cContext, [](){}, Qt::BlockingQueuedConnection);   ///< drain the queue
        QCOMPARE(iDeletedCalls.load(), 0);
        QCOMPARE(iKeptCalls.load(), 3);

        QMetaObject::invokeMethod(&cContext, [&](){ delete pcKept; }, Qt::BlockingQueuedConnection);
    }

    /// deleting the bus while events from it are still queued drops them safely
    void deletedBusDropsQueuedEvents()
    {
        GitlEventBus* pcBus = GitlEventBus::create();

        QThread cThread;
        QObject cContext;
        cContext.moveToThread(&cThread);
        cThread.start();
        auto cStop = MakeScopeExit([&](){ cThread.quit(); cThread.wait(); });

        std::atomic<int> iCalls(0);
        GitlModule* pcModule = new GitlModule(pcBus);
        pcModule->subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ ++iCalls; return true; });
        pcModule->moveModuleToThread(&cThread);

        /// park the worker so the events stay queued
        QSemaphore cPosted;
        QMetaObject::invokeMethod(&cContext, [&](){ cPosted.acquire(); }, Qt::QueuedConnection);

        for(int i = 0; i < 3; ++i)
            GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        delete pcBus;
        QVERIFY(pcModule->getEventBus() == nullptr);
        cPosted.release();

        QMetaObject::invokeMethod(&cContext, [](){}, Qt::BlockingQueuedConnection);   ///< drain the queue
        QCOMPARE(iCalls.load(), 0);

        QMetaObject::invokeMethod(&cContext, [&](){ delete pcModule; }, Qt::BlockingQueuedConnection);
    }

    /// the public eventTriggered() signal still reports every posted event to
    /// external listeners, including events nobody subscribed to
    void eventTriggeredSignal()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        QStringList vcSeen;
        QObject::connect(pcBus, &GitlEventBus::eventTriggered,
                         [&](QSharedPointer<GitlEvent> pcEvt){ vcSeen << pcEvt->getEvtName(); });

        TestModule cModule(pcBus);
        cModule.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModule, TestModule::callback));

        GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        GitlEvent("NOBODY_LISTENS").dispatch(pcBus);
        QVERIFY(cModule.getNotified());
        QCOMPARE(vcSeen, QStringList() << "TEST_EVENT_1" << "NOBODY_LISTENS");
    }

    /// subscriptions made while detached are routed once the module is attached,
    /// and a module that moves to another bus stops receiving from the old one
    void subscribeWhileDetached()
    {
        GitlEventBus* pcBus1 = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus1);
        GitlEventBus* pcBus2 = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus2);

        int iCalls = 0;
        GitlModule cModule(pcBus1);
        cModule.detach();
        cModule.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ ++iCalls; return true; });

        GitlEvent("TEST_EVENT_1").dispatch(pcBus1);
        QCOMPARE(iCalls, 0);

        cModule.attach(pcBus1);
        GitlEvent("TEST_EVENT_1").dispatch(pcBus1);
        QCOMPARE(iCalls, 1);

        cModule.attach(pcBus2);
        GitlEvent("TEST_EVENT_1").dispatch(pcBus1);
        QCOMPARE(iCalls, 1);
        GitlEvent("TEST_EVENT_1").dispatch(pcBus2);
        QCOMPARE(iCalls, 2);

        cModule.unsubscribeToEvtByName("TEST_EVENT_1");
        GitlEvent("TEST_EVENT_1").dispatch(pcBus2);
        QCOMPARE(iCalls, 2);
    }

    /// modules receive an event in the order they subscribed to it, and subscribing
    /// to other events later does not change that order
    void deliveryOrderIsStable()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        QString strOrder;
        GitlModule cFirst(pcBus), cSecond(pcBus);
        cFirst .subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ strOrder += "1"; return true; });
        cSecond.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ strOrder += "2"; return true; });

        cFirst.subscribeToEvtByName("OTHER_EVENT", [](const GitlEvent&){ return true; });
        cFirst.unsubscribeToEvtByName("OTHER_EVENT");

        GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        QCOMPARE(strOrder, QString("12"));
    }

    /// a callback may delete another module that is later in the same delivery
    void callbackDeletesOtherModule()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        int iFirst = 0, iSecond = 0;
        GitlModule cFirst(pcBus);
        GitlModule* pcSecond = new GitlModule(pcBus);
        cFirst.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&)
        {
            ++iFirst;
            delete pcSecond;
            pcSecond = nullptr;
            return true;
        });
        pcSecond->subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&){ ++iSecond; return true; });

        GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        QCOMPARE(iFirst, 1);
        QCOMPARE(iSecond, 0);
        QVERIFY(pcSecond == nullptr);
    }







    /// dispatchSync: a local event without CLONABLE reaches listeners on several
    /// threads by reference, and all of them have finished when the call returns
    void dispatchSyncCrossThread()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        const int iWorkers = 3;
        std::vector<std::unique_ptr<QThread>>    vcThreads;
        std::vector<std::unique_ptr<GitlModule>> vcModules;
        auto cCleanup = MakeScopeExit([&]()
        {
            for(auto& m : vcModules) m->detach();
            for(auto& t : vcThreads) { t->quit(); t->wait(); }
            vcModules.clear();
            vcThreads.clear();
        });

        NoCloneEvent cEvt(5);   ///< local, and no CLONABLE
        std::atomic<int> iCalls(0), iWrongObject(0), iWrongThread(0), iSum(0);
        auto fnListen = [&](GitlModule& rcModule, QThread* pcExpected)
        {
            rcModule.subscribeToEvtByName("NO_CLONE_EVENT", [&, pcExpected](const GitlEvent& rcEvt)
            {
                QThread::msleep(20);   ///< make sure the sender really waits
                if(&rcEvt != &cEvt) ++iWrongObject;
                if(QThread::currentThread() != pcExpected) ++iWrongThread;
                iSum += static_cast<const NoCloneEvent&>(rcEvt).m_iValue;
                ++iCalls;
                return true;
            });
        };

        GitlModule cLocal(pcBus);   ///< on this thread
        fnListen(cLocal, QThread::currentThread());
        for(int i = 0; i < iWorkers; ++i)
        {
            vcThreads.emplace_back(new QThread);
            vcThreads.back()->start();
            vcModules.emplace_back(new GitlModule(pcBus));
            fnListen(*vcModules.back(), vcThreads.back().get());
            vcModules.back()->moveModuleToThread(vcThreads.back().get());
        }

        int iTriggered = 0;
        QObject::connect(pcBus, &GitlEventBus::eventTriggered, [&](QSharedPointer<GitlEvent>){ ++iTriggered; });

        cLocal.dispatchSync(cEvt);
        QCOMPARE(iCalls.load(), iWorkers + 1);       ///< no waiting needed: all done already
        QCOMPARE(iSum.load(), 5 * (iWorkers + 1));
        QCOMPARE(iWrongObject.load(), 0);            ///< everyone saw the caller's object, not a copy
        QCOMPARE(iWrongThread.load(), 0);
        QCOMPARE(iTriggered, 0);                     ///< nothing may keep a pointer to the local

        cEvt.dispatchSync(pcBus);                    ///< the GitlEvent member works the same way
        QCOMPARE(iCalls.load(), 2 * (iWorkers + 1));
    }

    /// dispatchSync returns even if a receiving module is deleted before it handles the event
    void dispatchSyncReceiverDeleted()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        QThread cThread;
        QObject cContext;
        cContext.moveToThread(&cThread);
        cThread.start();
        auto cStop = MakeScopeExit([&](){ cThread.quit(); cThread.wait(); });

        std::atomic<int> iCalls(0);
        GitlModule* pcModule = new GitlModule(pcBus);
        pcModule->subscribeToEvtByName("NO_CLONE_EVENT", [&](const GitlEvent&){ ++iCalls; return true; });
        pcModule->moveModuleToThread(&cThread);

        /// queued ahead of the delivery: the module is gone before the event is handled
        QMetaObject::invokeMethod(&cContext, [&](){ QThread::msleep(50); delete pcModule; },
                                  Qt::QueuedConnection);
        NoCloneEvent cEvt(1);
        cEvt.dispatchSync(pcBus);   ///< must not hang
        QCOMPARE(iCalls.load(), 0);
    }

    /// a same-thread listener may dispatchSync again: it is called directly, no deadlock
    void dispatchSyncNested()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);
        GitlModule cModule(pcBus);

        int iOuter = 0, iInner = 0, iInnerSeenByOuter = -1;
        cModule.subscribeToEvtByName("OUTER", [&](const GitlEvent&)
        {
            ++iOuter;
            GitlEvent cInner("INNER");
            cModule.dispatchSync(cInner);
            iInnerSeenByOuter = iInner;   ///< already handled when dispatchSync returned
            return true;
        });
        cModule.subscribeToEvtByName("INNER", [&](const GitlEvent&){ ++iInner; return true; });

        GitlEvent cOuter("OUTER");
        cModule.dispatchSync(cOuter);
        QCOMPARE(iOuter, 1);
        QCOMPARE(iInner, 1);
        QCOMPARE(iInnerSeenByOuter, 1);
    }

    /// dispatchAsync copies the event and queues it, even for a listener on the same thread
    void dispatchAsyncIsQueued()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);
        GitlModule cModule(pcBus);

        int iCalls = 0;
        QString strSeen;
        cModule.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent& rcEvt)
        {
            ++iCalls;
            strSeen = static_cast<const CustomEvent&>(rcEvt).getCustomVar();
            return true;
        });

        CustomEvent cEvt("TEST_EVENT_1");          ///< has CLONABLE
        cModule.dispatchAsync(cEvt);
        QCOMPARE(iCalls, 0);                       ///< not yet: queued
        cEvt.setCustomVar("changed after dispatch");
        QTRY_COMPARE(iCalls, 1);
        QCOMPARE(strSeen, QString("Custom String"));   ///< a copy taken at dispatch time

        cEvt.dispatchAsync(pcBus);
        QCOMPARE(iCalls, 1);
        QTRY_COMPARE(iCalls, 2);
    }


#ifdef QT_NO_DEBUG
    /// dispatchAsync copies, so an event without CLONABLE is rejected (debug builds assert)
    void dispatchAsyncNeedsClone()
    {
        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);
        GitlModule cModule(pcBus);

        int iCalls = 0;
        cModule.subscribeToEvtByName("NO_CLONE_EVENT", [&](const GitlEvent&){ ++iCalls; return true; });

        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression("postAsync.*Add CLONABLE"));
        NoCloneEvent cEvt(1);
        cModule.dispatchAsync(cEvt);
        QTest::qWait(20);
        QCOMPARE(iCalls, 0);
    }
#endif

    /// several modules, each on its own worker thread, receive every event exactly once
    /// on their own thread while several threads dispatch and (un)subscribe concurrently
    void multipleWorkers()
    {
        const int iWorkers = 4;
        const int iSenders = 4;               ///< main thread + 3 std::threads
        const int iEventsPerSender = 200;
        const int iExpected = iSenders * iEventsPerSender;

        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        struct Stats { std::atomic<int> iCalls{0}; std::atomic<int> iWrongThread{0}; };
        std::vector<std::unique_ptr<QThread>>    vcThreads;
        std::vector<std::unique_ptr<Stats>>      vcStats;
        std::vector<std::unique_ptr<GitlModule>> vcModules;
        std::vector<std::thread>                 vcSenders;
        std::atomic<bool>                        bStop(false);
        std::thread                              cSubscriber;

        /// runs even if a QVERIFY fails, so no thread is destroyed while running
        auto cCleanup = MakeScopeExit([&]()
        {
            bStop = true;
            if(cSubscriber.joinable()) cSubscriber.join();
            for(auto& t : vcSenders) if(t.joinable()) t.join();
            for(auto& m : vcModules) m->detach();
            for(auto& t : vcThreads) { t->quit(); t->wait(); }
            vcModules.clear();
            vcThreads.clear();
        });

        for(int i = 0; i < iWorkers; ++i)
        {
            vcThreads.emplace_back(new QThread);
            vcThreads.back()->start();
            vcStats.emplace_back(new Stats);
            vcModules.emplace_back(new GitlModule(pcBus));

            QThread* pcThread = vcThreads.back().get();
            Stats* pcStats = vcStats.back().get();
            vcModules.back()->subscribeToEvtByName("TEST_EVENT_1", [=](const GitlEvent& rcEvt)
            {
                if(QThread::currentThread() != pcThread || rcEvt.getEvtName() != "TEST_EVENT_1")
                    ++pcStats->iWrongThread;
                ++pcStats->iCalls;
                return true;
            });
            vcModules.back()->moveModuleToThread(pcThread);
        }

        /// concurrent subscribe/unsubscribe of another event while events are flowing
        cSubscriber = std::thread([&]()
        {
            while(!bStop)
                for(auto& m : vcModules)
                {
                    m->subscribeToEvtByName("OTHER_EVENT", [](const GitlEvent&){ return true; });
                    m->unsubscribeToEvtByName("OTHER_EVENT");
                }
        });

        for(int s = 1; s < iSenders; ++s)
            vcSenders.emplace_back([=]()
            {
                for(int i = 0; i < iEventsPerSender; ++i)
                    GitlEvent("TEST_EVENT_1").dispatch(pcBus);
            });
        for(int i = 0; i < iEventsPerSender; ++i)
            GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        for(auto& t : vcSenders) t.join();

        for(int i = 0; i < iWorkers; ++i)
            QTRY_COMPARE_WITH_TIMEOUT(vcStats[i]->iCalls.load(), iExpected, 10000);

        bStop = true;
        cSubscriber.join();

        QTest::qWait(50);   ///< nothing extra may arrive later
        for(int i = 0; i < iWorkers; ++i)
        {
            QCOMPARE(vcStats[i]->iCalls.load(), iExpected);
            QCOMPARE(vcStats[i]->iWrongThread.load(), 0);
        }
    }

    /// a class can derive from both QObject and GitlModule: QObject's thread()
    /// and moveToThread() stay unambiguous, and both parts can be moved together
    void qobjectSubclass()
    {
        class Worker : public QObject, public GitlModule
        {
        public:
            explicit Worker(GitlEventBus* pcBus) : GitlModule(pcBus) {}
        };

        GitlEventBus* pcBus = GitlEventBus::create();
        SCOPE_EXIT(delete pcBus);

        QThread cThread;
        cThread.start();

        Worker cWorker(pcBus);
        std::atomic<QThread*> pcCallbackThread(nullptr);
        cWorker.subscribeToEvtByName("TEST_EVENT_1", [&](const GitlEvent&)
        {
            pcCallbackThread = QThread::currentThread();
            return true;
        });

        cWorker.moveToThread(&cThread);           ///< QObject part (unqualified call must compile)
        cWorker.moveModuleToThread(&cThread);     ///< GitlModule part
        QVERIFY(cWorker.thread() == &cThread);
        QVERIFY(cWorker.moduleThread() == &cThread);

        GitlEvent("TEST_EVENT_1").dispatch(pcBus);
        QTRY_VERIFY(pcCallbackThread.load() == &cThread);

        cThread.quit();
        cThread.wait();
        cWorker.detach();   ///< its thread has stopped; nothing more to deliver
    }

#ifdef QT_NO_DEBUG
    /// 1.2: an event subclass without CLONABLE is rejected instead of being sliced
    /// (debug builds assert instead, so this only runs in release)
    void missingClonableIsRejected()
    {
        class NotClonableEvent : public GitlEvent
        {
        public:
            NotClonableEvent() : GitlEvent("TEST_EVENT_1") {}
        };

        TestModule cModule;
        cModule.subscribeInsideClass();
        QTest::ignoreMessage(QtCriticalMsg, QRegularExpression("Add CLONABLE"));
        NotClonableEvent cEvt;
        cModule.dispatchEvt(cEvt);
        QVERIFY(!cModule.getNotified());
    }
#endif
};


/// test main
QTEST_MAIN(TestCase)
#include "testcase.moc"

