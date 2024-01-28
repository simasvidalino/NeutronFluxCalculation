#pragma once

#include <QObject>

class NJOYBasicTest : public QObject
{
    Q_OBJECT
public:
    NJOYBasicTest();

private slots:

    void testModules();

    QByteArray readTestNJOY(const QString fileName);
};

