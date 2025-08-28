#include "MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));

    MainWindow w;
    w.show();
    return a.exec();
}
