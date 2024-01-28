#pragma once

#include <QDoubleSpinBox>
#include <QGraphicsScene>
#include <QWidget>

#include <memory.h>
#include <optional>

#include "InterFaceDefinitions.h"
#include "VariablesUsed.h"

namespace Ui {
class RegionInputData;
}

class RegionInputData : public QWidget
{
    Q_OBJECT

public:
    explicit RegionInputData(QWidget *parent = nullptr);
    ~RegionInputData();

    void setGeneralProjectData(std::unique_ptr<Interface::projetData> &&proj);
    std::unique_ptr<Interface::projetData> &&getGeneralProjectData();

    std::shared_ptr<dados_entrada> getDdValues() const;

    int getRegionQuant() const;

public slots:
    //void onProjectSave(QString path);

private slots:
    void clear();

    void mapRegions();

    void onCreateCrossSectionFile();

    void onNJOYClicked();

    void onOpenBCLeftInputTable();

    void onOpenBCRightInputTable();

    void setGraphicScene(int region);

    void calculateEscalarNeutronFlux();

    void onSelectionRegionChange();

signals:
    void updateChartSignal();
    void updateProjectFiles();
    void updateGUI();

private:
    Ui::RegionInputData *ui;

    std::vector<int> calculateRegionHeights();

    void createSpinBoxes();

    void init();

    void setConnections();

    void setRegion(int regionNumber,
                   int left   = 0,
                   int top    = 0,
                   int width  = 50,
                   int height = 50);

    void setQuota(int regionNumber,
                  int left   = 0,
                  int top    = 0,
                  int width  = 50,
                  int height = 50);

    void setSpinBoxQuota(int regionNumber,
                         int left, int top,
                         int width,
                         int height);

    void setZonesLegend(int region);

    void updateDDValues();

    void loadGUI();

    void saveGUI();

    std::unique_ptr<QGraphicsScene> scene;

    std::array<Interface::regionData, 10> regionArray;

    std::array<QDoubleSpinBox*, 10> quoteSpinBoxes;

    int regionQuant;
    std::shared_ptr<dados_entrada> DDValues;

    QVector<QColor> zoneColors = {
        QColor(31, 120, 180),   // Azul
        QColor(51, 160, 44),    // Verde
        QColor(227, 26, 28),    // Vermelho
        QColor(255, 127, 0),    // Laranja
        QColor(106, 61, 154),   // Roxo
        QColor(177, 89, 40),    // Marrom
        QColor(166, 206, 227),  // Azul claro
        QColor(178, 223, 138),  // Verde claro
        QColor(251, 154, 153),  // Rosa claro
        QColor(253, 191, 111)   // Laranja claro
    };

    std::unique_ptr<Interface::projetData> proj;

    QStringList allZonasStr;
    QString scatteringPath = "";
    std::optional<std::vector<double>> bcLeft;
    std::optional<std::vector<double>> bcRight;
 };
