#pragma once
#include <QMainWindow>

#include "ProjectStructs.h"
#include "VariablesUsed.h"
#include "Worker.h"

#include <mutex>

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
    void onOutputData(std::shared_ptr<CalculatedData> data);
    void openProject();
    void openProjectFileDlg();
    void saveMaterialData();
    bool saveProject(bool saveMaterialDataFile = false);
    void saveProjectFileDlg();
    void showDataInFile(std::string &file);
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

    QString getDefaultName(const QString& name, const QString& extension);

    void showInvalidZoneMessage();

    void init();

    void showDefaultProjectWarning();

    void setConnections();

    void startWork();
    void stopWork();

    void updateGUIWithCalculatedData();

    ProjectData projectData;

    double periodicityValue = 10.0;

    QString fileName;

    std::shared_ptr<ProjectData> proj;
    std::shared_ptr<CalculatedData> calculatedData;

    QThread* calculationThread;
    Worker* worker;

    std::mutex m_dataMutex;
    std::mutex m_projMutex;
};
