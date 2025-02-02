#pragma once
#include <QMainWindow>

#include "ProjectStructs.h"
#include "VariablesUsed.h"
#include "Worker.h"

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
    void changePalette();
    void openProject();
    void openProjectFileDlg();
    bool saveProject();
    void saveProjectFileDlg();
    void showCrossSectionFile(std::string &file);
    void updateAbsRateChart(std::shared_ptr<CalculatedData> DDResult);
    void updateAbsRateTable(std::shared_ptr<CalculatedData> DDResult);
    void updateFluxChart(std::shared_ptr<CalculatedData> DDResult);
    void updateFluxTable(std::shared_ptr<CalculatedData> DDResult);

signals:
    void startProcess();
private:
    enum tabs
    {
        tabInputData,
        TabResult
    };

    Ui::MainWindow *ui;

    void enableGenerateFilesMenu();

    void init();

    void showDefaultProjectWarning();

    void setConnections();

    void startWork();
    void stopWork();

    ProjectData projectData;

    double periodicityValue = 10.0;

    QString fileName;

    std::unique_ptr<ProjectData> proj;

    QThread* calculationThread;
    Worker* worker;
};
