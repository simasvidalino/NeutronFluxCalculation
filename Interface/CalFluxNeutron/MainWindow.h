#pragma once
#include <QMainWindow>

#include "InterFaceDefinitions.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void changeFont();
    void changeViewMode();
    void changePaletteToDarkStyle();
    void openProject();
    void saveProject();
    void updateChart();

signals:

private:
    enum tabs
    {
        tabInputData,
        TabResult
    };

    Ui::MainWindow *ui;

    void init();

    void setConnections();


    int periodicityValue = 10;
 };
