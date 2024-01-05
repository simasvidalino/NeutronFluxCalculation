#pragma once

#include <QDoubleSpinBox>
#include <QGraphicsScene>
#include <QWidget>

#include <memory.h>

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

    std::shared_ptr<dados_entrada> getDdValues() const;

private slots:
    void clear();

    void setGraphicScene(int region);

    void calculateEscalarNeutronFlux();

signals:
    void updateChartSignal();

private:
    Ui::RegionInputData *ui;

    struct regionData
    {
        int region;
        int quote;
    };

    std::vector<int> calculateRegionHeights();

    void createSpinBoxes();

    void init();

    void setConnections();

    void setRegion(int regionNumber,
                   int left   = 0,
                   int top    = 0,
                   int width  = 50,
                   int height = 50);

    void setQuota(int regionNumber, int left   = 0,
                  int top    = 0,
                  int width  = 50,
                  int height = 50);

    void setSpinBoxQuota(int regionNumber, int left, int top, int width, int height);

    std::unique_ptr<QGraphicsScene> scene;

    std::array<regionData, 10> regionArray;

    std::array<QDoubleSpinBox*, 10> quoteSpinBoxes;

    int regionQuant;
    std::shared_ptr<dados_entrada> DDValues;
};
