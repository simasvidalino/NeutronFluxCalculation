#pragma once

#include "qlabel.h"
#include <QComboBox>
#include <QLineSeries>
#include <QtCharts/QChartView>
#include <QLocale>
#include <QSpinBox>

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

    void setGroup(int newGroup);

    void setInputData(QList<QPointF> &points, int group = 0);

    QChart *getChart();

    void setXLabel(const QString &name);

    void setYLabel(const QString &name);

    void setProjectionTitle(const QString value);

    int getPeriodicity();
    void setPeriodicity(int value);

    void setTickNumber(int newTickNumber);

    void setYRange(int min, int max);

    void setXRange(int min, int max);

    void setFilterByGroup();

    void showPeriodicity();

public slots:
    void setChart();

private slots:
    void filterChange(int option);

signals:
    void updatePeriodicity(int);

private:

    void init();
    void setConnection();

    QString projectionTitle;
    QString xLabel;
    QString yLabel;
    QString unit;

    QLocale locale;

    double maxY = 1;
    int tickNumber;

    QValueAxis* axisX;
    QValueAxis* axisY;

    std::map<int, QLineSeries*> seriesByGroup;

    QList<int> regionSize;

    QList<QColor> colors = {Qt::blue, Qt::red, Qt::cyan};

    QComboBox* filter = nullptr;
    QSpinBox* periodicity = nullptr;
    QLabel* periodicityLabel = nullptr;

    int option = 0;

    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
};

