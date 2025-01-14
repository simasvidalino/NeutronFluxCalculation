#pragma once

#include <QComboBox>
#include <QLineSeries>
#include <QtCharts/QChartView>
#include <QLabel>
#include <QLocale>
#include <QDoubleSpinBox>

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

    void hideLegend();

    void setGroup(int newGroup);

    void setInputData(QList<QPointF> &value, int group);
    void setInputData(QList<QPointF> &value);

    QChart *getChart();

    void setXLabel(const QString &name);

    void setYLabel(const QString &name);

    void setProjectionTitle(const QString value);

    void setTickNumber(int newTickNumber);

    void setYRange(int min, int max);

    void setXRange(int min, int max);

public slots:
    void setChart();
    void filterChange(int option);

signals:
    void updatePeriodicity(double);

private:

    void init();
    void setConnection();

    QString projectionTitle;
    QString xLabel;
    QString yLabel;
    QString unit;

    QLocale locale;

    int tickNumber;

    QValueAxis* axisX;
    QValueAxis* axisY;

    std::map<int, QLineSeries*> seriesByGroup;

    QList<int> regionSize;

    QList<QColor> colors = {Qt::blue, Qt::red, Qt::cyan};

    int option = 0;

    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
};

