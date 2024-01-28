#pragma once

#include <QSplashScreen>

namespace Ui {
class NJoySplashScreen;
}

class NJoySplashScreen : public QSplashScreen
{
    Q_OBJECT

public:
    explicit NJoySplashScreen(QWidget *parent = nullptr);
    ~NJoySplashScreen();

    virtual void show();

private:
    Ui::NJoySplashScreen *ui;

    void setTimer();
};
