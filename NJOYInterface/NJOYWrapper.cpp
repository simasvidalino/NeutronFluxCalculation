#include "NJOYWrapper.h"

#include <QDebug>
#include <QProcess>

NJOYWrapper::NJOYWrapper(QObject *parent) : QObject(parent)
{

}

NJOYWrapper::~NJOYWrapper()
{

}

QByteArray NJOYWrapper::runNjoy(const QString &njoyExecutable, const QStringList &arguments)
{
        QProcess process(this);
        process.start(njoyExecutable, arguments);

        if (!process.waitForFinished())
            qDebug() << "Make failed:" << process.errorString();
        else
            qDebug() << "Make output:" << process.readAll();

        return process.readAllStandardOutput();
}
