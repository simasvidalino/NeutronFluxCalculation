#pragma once

#include <QMainWindow>

#include "NJOYEnterDataWizard.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void openFile(const QString &fileName);

    void setConnections();

    NJOYEnterDataWizard wizard;
};
