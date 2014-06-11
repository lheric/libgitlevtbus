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
#include "gitlmodual.h"
#include "gitleventbus.h"
using namespace std;

///
class TestModual : public GitlModual
{
public:
    TestModual()
    {
        this->m_bNotified = false;

    }

    void subscribeInsideClass()
    {
        subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK(TestModual::callback));
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

class CustomEventListener : public GitlModual
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
        TestModual cModual;
        cModual.subscribeToEvtByName("TEST_EVENT_1",
         [&](GitlEvent& e)->bool
         {
            cModual.setNotified(true);
            return true;
         });
        QVERIFY(!cModual.getNotified());
        GitlEvent cEvt("TEST_EVENT_1");
        cModual.dispatchEvt(cEvt);
        QVERIFY(cModual.getNotified());
    }


    void listenInsideClass()
    {
        TestModual cModual;
        cModual.subscribeInsideClass();
        QVERIFY(!cModual.getNotified());
        GitlEvent cEvt("TEST_EVENT_1");
        cEvt.dispatch();
        QVERIFY(cModual.getNotified());
    }

    void listenOutsideClass()
    {
        TestModual cModual;
        cModual.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModual, TestModual::callback));
        QVERIFY(!cModual.getNotified());
        GitlEvent cEvt("TEST_EVENT_1");
        cModual.dispatchEvt(cEvt);
        QVERIFY(cModual.getNotified());
    }

    void unsubscribe()
    {
        TestModual cModual;
        cModual.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModual, TestModual::callback));
        cModual.unsubscribeToEvtByName("TEST_EVENT_1");
        QVERIFY(!cModual.getNotified());
        GitlEvent cEvt("TEST_EVENT_1");
        cModual.dispatchEvt(cEvt);
        QVERIFY(!cModual.getNotified());
    }



    void oneToMany()
    {
        TestModual cSender;
        TestModual cModual1;
        TestModual cModual2;
        TestModual cModual3;
        cModual1.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModual1, TestModual::callback));
        cModual2.subscribeToEvtByName("TEST_EVENT_1", MAKE_CALLBACK_OBJ(cModual2, TestModual::callback));
        cModual3.subscribeToEvtByName("TEST_EVENT_2", MAKE_CALLBACK_OBJ(cModual3, TestModual::callback));

        GitlEvent cEvt1("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt1);
        QVERIFY(cModual1.getNotified());
        QVERIFY(cModual2.getNotified());
        QVERIFY(!cModual3.getNotified());

        GitlEvent cEvt2("TEST_EVENT_2");
        cSender.dispatchEvt(cEvt2);
        QVERIFY(cModual3.getNotified());

    }

    void customEventTest()
    {
        CustomEventListener cModual;
        cModual.subscribeToEvtByName("TEST_EVENT_1",MAKE_CALLBACK_OBJ(cModual, CustomEventListener::callback));
        CustomEvent cEvt("TEST_EVENT_1");
        cEvt.dispatch();
        QVERIFY(cModual.getNotified());
        QVERIFY(cModual.getCustomVar() == QString("Custom String"));
    }
};


/// test main
QTEST_MAIN(TestCase)
#include "testcase.moc"

