#pragma once

#include <QDoubleSpinBox>
#include <QGraphicsScene>
#include <QWidget>

#include <memory.h>
#include <optional>

#include <ProjectStructs.h>
#include "VariablesUsed.h"
#include "Worker.h"

namespace Ui
{
    class RegionInputData;
}

class RegionInputData : public QWidget
{
    Q_OBJECT

public:
    explicit RegionInputData(QWidget *parent = nullptr);
    ~RegionInputData();

    void setGeneralProjectData(std::unique_ptr<ProjectData> &&proj);
    std::unique_ptr<ProjectData> &&getGeneralProjectData();

    std::shared_ptr<dados_entrada> getDdValues() const;

    const std::vector<std::vector<long double>> &getScalarFlux() const;

    int getRegionQuant() const;
    int getNumberOfGroup();

    std::shared_ptr<CalculatedData> getDDOutputValues() const;

    void setPeriodicity(double newPeriodicity);

    void setPushButtonCalculateFluxEnable(bool enable);

private slots:
    void clear();
    void onCreateCrossSectionFile();
    void onNJOYClicked();
    void onOpenBCLeftInputTable();
    void onOpenBCRightInputTable();
    void setGraphicScene(int region);
    void onSelectionRegionChange();

signals:
    void updateProjectFiles();
    void onCalculateScalarNeutronFlux();

private:
    Ui::RegionInputData *ui;

    std::vector<int> calculateRegionHeights();

    void createSpinBoxes();

    void init();

    void setConnections();

    void setRegionGraphicsRectItem(int regionNumber,
                                   int left = 0,
                                   int top = 0,
                                   int width = 50,
                                   int height = 50);

    void setQuotaLinesGraphicsItem(int regionNumber,
                                   int left = 0,
                                   int top = 0,
                                   int width = 50,
                                   int height = 50);

    void setSpinBoxQuota(int regionNumber,
                         int left, int top,
                         int width,
                         int height);

    void styleSpinBoxQuota(QDoubleSpinBox *quota);

    void loadGUI();

    void saveGUI();

    std::unique_ptr<QGraphicsScene> scene;
    std::array<RegionData, 10> regionArray;
    std::array<QDoubleSpinBox *, 10> quoteSpinBoxes;

    int regionQuant;
    std::shared_ptr<dados_entrada> DDValues;
    std::shared_ptr<CalculatedData> DDOutputValues;
    std::vector<std::vector<long double>> scalarFlux;

    QVector<QColor> zoneColors = {
        QColor(143, 187, 217), // Pastel Blue
        QColor(153, 207, 149), // Pastel Green
        QColor(241, 140, 141), // Pastel Red
        QColor(255, 191, 127), // Pastel Orange
        QColor(180, 158, 204), // Pastel Purple
        QColor(216, 172, 147), // Pastel Brown
        QColor(211, 230, 241), // Light Pastel Blue
        QColor(214, 239, 196), // Light Pastel Green
        QColor(251, 204, 202), // Light Pastel Pink
        QColor(253, 223, 182)  // Light Pastel Orange
    };

    std::unique_ptr<ProjectData> proj;

    QStringList allZonasStr;
    QString scatteringPath = "";
    std::optional<std::vector<double>> bcLeft;
    std::optional<std::vector<double>> bcRight;
    double periodicity;
};
