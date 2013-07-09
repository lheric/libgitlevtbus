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

    virtual bool detonate( QSharedPointer<GitlEvent> pcEvt )
    {
        this->m_bNotified = true;
        return true;
    }

    ADD_CLASS_FIELD_NOSETTER(bool, bNotified, getNotified)
};


/// custom event
class CustomEvent : public GitlEvent
{
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

    virtual bool detonate( QSharedPointer<GitlEvent> pcEvt )
    {
        QSharedPointer<CustomEvent> pcCusEvt = pcEvt.staticCast<CustomEvent>();
        this->m_bNotified = true;
        this->m_strCustomVar = pcCusEvt->getCustomVar();
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
        QSharedPointer<GitlEvent> pcEvt ( new GitlEvent("TEST_EVENT_1"));
        cSender.dispatchEvt(pcEvt);
        QVERIFY(cListener.getNotified());
    }

    void selfSendAndListen()
    {
        TestModual cSender;
        cSender.subscribeToEvtByName("TEST_EVENT_1");
        QSharedPointer<GitlEvent> pcEvt ( new GitlEvent("TEST_EVENT_1") );
        cSender.dispatchEvt(pcEvt);
        QVERIFY(cSender.getNotified());
    }

    void unsubscribe()
    {
        TestModual cSender;
        cSender.subscribeToEvtByName("TEST_EVENT_1");
        cSender.unsubscribeToEvtByName("TEST_EVENT_1");
        QSharedPointer<GitlEvent> pcEvt ( new GitlEvent("TEST_EVENT_1") );
        cSender.dispatchEvt(pcEvt);
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

        QSharedPointer<GitlEvent> pcEvt1 ( new GitlEvent("TEST_EVENT_1") );
        cSender.dispatchEvt(pcEvt1);
        QVERIFY(cListener1.getNotified());
        QVERIFY(cListener2.getNotified());
        QVERIFY(!cListener3.getNotified());

        QSharedPointer<GitlEvent> pcEvt2 ( new GitlEvent("TEST_EVENT_2") );
        cSender.dispatchEvt(pcEvt2);
        QVERIFY(cListener3.getNotified());

    }

    void customEventTest()
    {
        TestModual cSender;
        CustomEventListener cListener;
        cListener.subscribeToEvtByName("TEST_EVENT_1");
        QSharedPointer<GitlEvent> pcEvt ( new CustomEvent("TEST_EVENT_1"));
        cSender.dispatchEvt(pcEvt);
        QVERIFY(cListener.getNotified());
        QVERIFY(cListener.getCustomVar() == QString("Custom String"));
    }


};


/// test main
QTEST_MAIN(TestCase)
#include "testcase.moc"

