#pragma once

#include <QDialog>

namespace Ui {
class NJOYStackWidget;
}

class NJOYStackWidget : public QDialog
{
    Q_OBJECT

public:
    explicit NJOYStackWidget(QWidget *parent = nullptr);
    ~NJOYStackWidget();

    void addWidget(QWidget *widget, const QString moduleName = "");
    void setCurrentIndex(int index);
    QWidget * getWidget(int index);

private:
    Ui::NJOYStackWidget *ui;

    void help();

    void setConnections();

    QStringList matNumbers;
};
