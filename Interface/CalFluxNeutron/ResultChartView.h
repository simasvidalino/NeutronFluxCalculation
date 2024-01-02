#pragma once

#include <QtCharts/QChartView>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

class ChartView : public QChartView {
    Q_OBJECT

public:
    ChartView(QWidget *parent = nullptr);

//private:
//    QtCharts::QChart *chart;
    //QtCharts::QBarSeries *series;
};

