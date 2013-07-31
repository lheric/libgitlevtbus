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

    virtual bool detonate( GitlEvent& rcEvt)
    {
        this->m_bNotified = true;
        return true;
    }

    ADD_CLASS_FIELD_NOSETTER(bool, bNotified, getNotified)
};


/// custom event
class CustomEvent : public GitlEvent
{
    VIRTUAL_COPY_PATTERN(CustomEvent)
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

    virtual bool detonate( GitlEvent& rcEvt)
    {
        CustomEvent& pcCusEvt = static_cast<CustomEvent&>(rcEvt);
        this->m_bNotified = true;
        this->m_strCustomVar = pcCusEvt.getCustomVar();
        return true;
    }

    ADD_CLASS_FIELD_NOSETTER(bool, bNotified, getNotified)
    ADD_CLASS_FIELD(QString, strCustomVar, getCustomVar, setCustomVar)
};



/// test case
class TestCase : public QObject
{
    Q_OBJECT

private slots:
    void oneSendAnotherListen()
    {

        TestModual cSender;
        TestModual cListener;
        cListener.subscribeToEvtByName("TEST_EVENT_1");
        GitlEvent pcEvt("TEST_EVENT_1");
        cSender.dispatchEvt(pcEvt);
        QVERIFY(cListener.getNotified());
    }

    void selfSendAndListen()
    {
        TestModual cSender;
        cSender.subscribeToEvtByName("TEST_EVENT_1");
        GitlEvent cEvt("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt);
        QVERIFY(cSender.getNotified());
    }

    void unsubscribe()
    {
        TestModual cSender;
        cSender.subscribeToEvtByName("TEST_EVENT_1");
        cSender.unsubscribeToEvtByName("TEST_EVENT_1");
        GitlEvent cEvt("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt);
        QVERIFY(!cSender.getNotified());
    }



    void oneToMany()
    {
        TestModual cSender;
        TestModual cListener1;
        TestModual cListener2;
        TestModual cListener3;
        cListener1.subscribeToEvtByName("TEST_EVENT_1");
        cListener2.subscribeToEvtByName("TEST_EVENT_1");
        cListener3.subscribeToEvtByName("TEST_EVENT_2");

        GitlEvent cEvt1("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt1);
        QVERIFY(cListener1.getNotified());
        QVERIFY(cListener2.getNotified());
        QVERIFY(!cListener3.getNotified());

        GitlEvent cEvt2("TEST_EVENT_2");
        cSender.dispatchEvt(cEvt2);
        QVERIFY(cListener3.getNotified());

    }

    void customEventTest()
    {
        TestModual cSender;
        CustomEventListener cListener;
        cListener.subscribeToEvtByName("TEST_EVENT_1");
        CustomEvent cEvt("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt);
        QVERIFY(cListener.getNotified());
        QVERIFY(cListener.getCustomVar() == QString("Custom String"));
    }


    void selfDispatchTest()
    {
        CustomEventListener cListener;
        cListener.subscribeToEvtByName("TEST_EVENT_1");
        CustomEvent("TEST_EVENT_1").dispatch();
        QVERIFY(cListener.getNotified());
        QVERIFY(cListener.getCustomVar() == QString("Custom String"));        
    }


};


/// test main
QTEST_MAIN(TestCase)
#include "testcase.moc"

