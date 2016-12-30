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
#include "gitldef.h"
#include "gitlmodule.h"
#include "gitleventbus.h"
using namespace std;

///
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

    bool callback( GitlEvent& rcEvt)
    {
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

class CustomEventListener : public GitlModule
{
public:
    CustomEventListener()
    {
        this->m_bNotified = false;
    }

    bool callback( GitlEvent& rcEvt)
    {
        CustomEvent& pcCusEvt = static_cast<CustomEvent&>(rcEvt);
        this->m_bNotified = true;
        this->m_strCustomVar = pcCusEvt.getCustomVar();
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
         [&](GitlEvent& e)->bool
         {
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
        GitlEventBus* pcBus1 = GitlEventBus::create(); TestModule cModule1(pcBus1); TestModule cModule2(pcBus1);
        GitlEventBus* pcBus2 = GitlEventBus::create(); TestModule cModule3(pcBus2); TestModule cModule4(pcBus2);

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

    }
};


/// test main
QTEST_MAIN(TestCase)
#include "testcase.moc"

