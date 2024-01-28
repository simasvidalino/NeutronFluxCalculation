#include "NJoySplashScreen.h"
#include "build/ui_NJoySplashScreen.h"
#include "qscreen.h"

#include <QTimer>
#include <QStyle>

NJoySplashScreen::NJoySplashScreen(QWidget *parent) :
    QSplashScreen(QPixmap()),
    ui(new Ui::NJoySplashScreen)
{
    ui->setupUi(this);

    this->setGeometry(
                QStyle::alignedRect(
                          Qt::LeftToRight, Qt::AlignCenter, this->size(),
                    QGuiApplication::screens().at(0)->geometry()));
}

NJoySplashScreen::~NJoySplashScreen()
{
    delete ui;
}

void NJoySplashScreen::show()
{
    QSplashScreen::show();
    setTimer();
}

void NJoySplashScreen::setTimer()
{
    QTimer::singleShot(4000, this, &NJoySplashScreen::close);

}
