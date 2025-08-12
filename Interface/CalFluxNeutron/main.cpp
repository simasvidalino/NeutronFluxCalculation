#include "MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));

    for (const QString &style : QStyleFactory::keys()) {
        qDebug() << "Available style:" << style;
    }
    MainWindow w;
    w.show();
    return a.exec();
}
