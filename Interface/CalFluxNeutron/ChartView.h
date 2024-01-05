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

    void clearChart();

    void setInputData(QList<QPointF> &points, QList<int> &regions, int group);

    QChart *getChart();

    void setXLabel(const QString &name);

    void setYLabel(const QString &name);

    void setProjectionTitle(const QString value);

    void setTickNumber(int newTickNumber);

public slots:
    void setChart();

private:

    QString projectionTitle;
    QString xLabel;
    QString yLabel;
    QString unit;

    QLocale locale;

    double maxY = 1;
    int tickNumber;

    QChart* chart;

    QValueAxis* axisX;
    QValueAxis* axisY;

    std::map<int, QLineSeries*> seriesByGroup;

    QList<int> regionSize;

    QList<QColor> colors = {Qt::blue, Qt::red, Qt::cyan};

};

