#pragma once

#include "qlineseries.h"
#include <QtCharts/QChartView>

#include <QLocale>

#include <memory>

class QLineSeries;
class QValueAxis;

class ChartView : public QChartView
{
    Q_OBJECT
public:
    explicit ChartView(QWidget *parent = nullptr);

    ~ChartView();

    void setInputData(QList<QPointF> &points, int group);

    QChart *getChart();

    void setXLabel(const QString &name);

    void setYLabel(const QString &name);

    void setProjectionTitle(const QString value);

    void setTickNumber(int newTickNumber);

public slots:
    void setChart();

private:
    void clearChart();

    void setAxes();

    QString projectionTitle;
    QString xLabel;
    QString yLabel;
    QString unit;

    QLocale locale;

    double maxY = 1;
    int tickNumber;

    std::unique_ptr<QChart> chart;

    std::unique_ptr<QValueAxis> axisX;
    std::unique_ptr<QValueAxis> axisY;

    std::map<int, std::unique_ptr<QLineSeries>> seriesByGroup;
};

