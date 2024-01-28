#include "NJOYBasicTest.h"
#include "QtTest/qtest.h"

#include <QFile>
#include <QDebug>
#include <QtTest/QTest>

NJOYBasicTest::NJOYBasicTest() : QObject()
{

}

void NJOYBasicTest::testModules()
{
    auto input       = readTestNJOY(":/tests/input1");
    auto inputByUser = readTestNJOY(":/tests/input26");

    QTest::addColumn<QString>("dataFromInterface");
    QTest::addColumn<QString>("expectedData");

    QTest::newRow("Test1") << QString::fromUtf8(input)
                           << QString::fromUtf8(inputByUser);

    QCOMPARE(input, inputByUser);
}

QByteArray NJOYBasicTest::readTestNJOY(const QString fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qInfo()<<"File Error";
    }

    QByteArray line;

    while (!file.atEnd())
    {
        line = file.readLine();
    }

    return line;
}


QTEST_MAIN(NJOYBasicTest)
#include "NJOYBasicTest.moc"

