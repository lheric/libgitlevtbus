#include <QCoreApplication>
#include <iostream>
#include <QtTest/QtTest>
#include <QTest>
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

    virtual bool detonate( GitlEvent cEvt )
    {
        this->m_bNotified = true;
        return true;
    }

    ADD_CLASS_FIELD_NOSETTER(bool, bNotified, getNotified)
};


class NestEventTestModual : public GitlModual
{
public:
    NestEventTestModual()
    {
        this->m_bOuter = false;
        this->m_bInner = false;
        subscribeToEvtByName("OUTER_EVENT_1");
        subscribeToEvtByName("INNER_EVENT_1");
    }

    virtual bool detonate( GitlEvent cEvt )
    {
        if(cEvt.getEvtName() == "OUTER_EVENT_1")
        {
            this->m_bOuter = true;
            GitlEvent cNestEvt; cNestEvt.setEvtName("INNER_EVENT_1");
            dispatchEvt(cNestEvt);
        }
        if(cEvt.getEvtName() == "INNER_EVENT_1")
        {
            this->m_bInner = true;
        }
        return true;
    }

    ADD_CLASS_FIELD_NOSETTER(bool, bOuter, getOuter)
    ADD_CLASS_FIELD_NOSETTER(bool, bInner, getInner)
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
        GitlEvent cEvt("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt);
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

        GitlEvent cEvt("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt);
        QVERIFY(cListener1.getNotified());
        QVERIFY(cListener2.getNotified());
        QVERIFY(!cListener3.getNotified());

        cEvt.setEvtName("TEST_EVENT_2");
        cSender.dispatchEvt(cEvt);
        QVERIFY(cListener3.getNotified());

    }

    void nestedEvent()
    {
        TestModual cSender;
        NestEventTestModual cListener;
        GitlEvent cEvt("OUTER_EVENT_1");
        cSender.dispatchEvt(cEvt);
        QVERIFY(cListener.getOuter());
        QVERIFY(cListener.getInner());
    }



};


/// test main
QTEST_MAIN(TestCase)
#include "testcase.moc"

