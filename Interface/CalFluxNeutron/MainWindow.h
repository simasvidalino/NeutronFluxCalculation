#pragma once
#include <QMainWindow>

#include "ProjectStructs.h"
#include "VariablesUsed.h"

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
    void calculateNeutronFluxUsingDD();
    void changeFont();
    void changeViewMode();
    void changePaletteToDarkStyle();
    void openProject();
    void saveProjectDlg();
    void saveProject();
    void updateAbsRateChart(std::shared_ptr<CalculatedData> DDResult);
    void updateFluxChart(std::shared_ptr<CalculatedData> DDResult);
    void updateChartStep();

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

    ProjectData projectData;

    double periodicityValue = 10.0;

    QString fileName;

    std::unique_ptr<ProjectData> proj;
 };
