#include "MainWindow.h"

#include <memory.h>
#include "NJoySplashScreen.h"

#include <QApplication>
#include <QScreen>
#include <QTimer>

#include "NJOYChangeNJOYOutputFormat.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QPalette palette;
//    auto bk = QPixmap(":/Icons/jeremybishopunsplash.jpg");
//    palette.setBrush(QPalette::Window, bk);
//    palette.setBrush(QPalette::Highlight, QColor( 144, 175, 175));
//    palette.setBrush(QPalette::HighlightedText, QColor( 144, 175, 175));
//    palette.setBrush(QPalette::LinkVisited, Qt::blue);
//    palette.setBrush(QPalette::Base, Qt::gray);
//    palette.setBrush(QPalette::Text, Qt::cyan);
//    palette.setBrush(QPalette::WindowText, Qt::white);
//    palette.setBrush(QPalette::ButtonText, Qt::cyan);
//    palette.setBrush(QPalette::Button, QColor( 144, 175, 175));
//    palette.setBrush(QPalette::HighlightedText, Qt::color1);
//    palette.setBrush(QPalette::Dark, Qt::darkGray);
//    palette.setColor(QPalette::Disabled, QPalette::Button, Qt::darkGray);
//    //mSplashScreen.setPalette(palette);

    //    a.setPalette(palette);

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    a.setPalette(darkPalette);

    NJoySplashScreen mSplashScreen;
    auto size = QApplication::primaryScreen()->size();
    mSplashScreen.setGeometry(QRect (QPoint(size.width()/2, 100), QSize(size.width()/2,  size.height()/2)));

    mSplashScreen.show();

    MainWindow w;

    QTimer::singleShot(4000, &w, &MainWindow::show);

    return a.exec();

//    NJOYChangeNJOYOutputFormat obj;

//    obj.read("/home/andreiasimas/Área de trabalho/NJOY2016/build/tape28", "525");

//    auto cs = obj.getTotalInDescendingOrder();
}
