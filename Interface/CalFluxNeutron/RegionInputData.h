#pragma once

#include <QDoubleSpinBox>
#include <QGraphicsScene>
#include <QWidget>

#include <memory.h>

namespace Ui {
class RegionInputData;
}

class RegionInputData : public QWidget
{
    Q_OBJECT

public:
    explicit RegionInputData(QWidget *parent = nullptr);
    ~RegionInputData();

private:
    Ui::RegionInputData *ui;

    struct regionData
    {
        int region;
        int quote;
    };

    std::vector<int> calculateRegionHeights();

    void clear();

    void calculateEscalarNeutronFlux();

    void createSpinBoxes();

    void init();

    void setConnections();

    void setGraphicScene(int region);

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
};
