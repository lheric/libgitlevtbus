#include <QDebug>
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include "gitldef.h"

/// test case
class ExampleClass : public QObject
{
    Q_OBJECT



public:
    ExampleClass()
    {
    }

signals:

    ADD_QPROP_PR(int, iIntPR )
    ADD_QPROP_PR_INIT(int, iIntPRInit, 123)

    ADD_QPROP_RO(QString, strRO, getRO)
    ADD_QPROP_RO_INIT(QString, strROInit, getROInit, "ROInit")

    ADD_QPROP_RW(QString, strRW, getRW, setRW)
    ADD_QPROP_RW_INIT(QString, strRWInit, getRWInit, setRWInit, "RWInit")

    ADD_FIELD(int, iIntField, getIntField, setIntField)
    ADD_FIELD_INIT(int, iIntFieldInit, getIntFieldInit, setIntFieldInit, 987)

};



/// test macro
class TestMacro: public QObject
{
    Q_OBJECT
public:
    TestMacro(){}

private slots:

    void test_ADD_QPROP_PR()
    {
        ExampleClass exampleClass;
        QCOMPARE(exampleClass.property("iIntPR").isValid(), true);
        QCOMPARE(exampleClass.property("iIntPRInit").isValid(), true);
        QCOMPARE(exampleClass.property("iIntPRInit").toInt(), 123);

        QCOMPARE(exampleClass.property("strRO").isValid(), true);
        QCOMPARE(exampleClass.property("strROInit").isValid(), true);
        QCOMPARE(exampleClass.getRO(), QString(""));
        QCOMPARE(exampleClass.getROInit(), QString("ROInit"));

        QCOMPARE(exampleClass.property("strRW").isValid(), true);
        QCOMPARE(exampleClass.property("strRWInit").isValid(), true);
        QCOMPARE(exampleClass.getRW(), QString(""));
        QCOMPARE(exampleClass.getRWInit(), QString("RWInit"));
        exampleClass.getRWInit() = "Modified";
        QCOMPARE(exampleClass.getRWInit(), QString("Modified"));

        exampleClass.getIntField();
        QCOMPARE(exampleClass.getIntFieldInit(), 987);
    }

    void test_signals()
    {
        ExampleClass exampleClass;

        bool bTriggered = false;
        connect(&exampleClass, &ExampleClass::strRWInitChanged, [&](QString &strRWInitChanged){bTriggered = true;});
        exampleClass.setProperty("strRWInit","changed");

        QCOMPARE(exampleClass.getRWInit(), QString("changed"));

    }



};



/// test main
QTEST_MAIN(TestMacro)
#include "testmacro.moc"
