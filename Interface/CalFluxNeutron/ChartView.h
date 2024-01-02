#pragma once

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

    void setInputData(double &maxY, std::unique_ptr<QLineSeries> &&value);

    QChart *getChart();

    void setGradesLabel(const QString &name);

    void setProjectionTitle(const QString value);

public slots:
    void setChart();

private:
    QString projectionTitle;
    QString gradeLabel;
    QString unit;

    QLocale locale;

    double maxY;

    std::unique_ptr<QChart> chart;

    std::unique_ptr<QValueAxis> axisX;
    std::unique_ptr<QValueAxis> axisY;

    std::unique_ptr<QLineSeries> series;;

    void clearChart();

    void setAxes();
};

