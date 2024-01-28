#pragma once

#include <QObject>

class NJOYWrapper : public QObject
{
public:
    explicit NJOYWrapper(QObject *parent = nullptr);
    ~NJOYWrapper();

    QByteArray runNjoy(const QString &njoyExecutable, const QStringList &arguments);
};
