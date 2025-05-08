#pragma once

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QMap>

struct SeriesData
{
    QList<QPointF> points;
    QColor color;
};

class ChartView : public QChartView
{
    Q_OBJECT
public:
    explicit ChartView(QWidget *parent = nullptr);
    ~ChartView();

    void clearChart();
    void clearData();
    void hideLegend();

    void setInputData(const QList<QPointF> &points, int group, QColor color = QColor());

    void setXLabel(const QString &label);
    void setYLabel(const QString &label);
    void setProjectionTitle(const QString &title);

    void setTickNumber(int newTickNumber);
    void setYRange(double min, double max);
    void setXRange(double min, double max);

public slots:
    void setChart();
    void filterChange(int option);

private:
    void init();
    void addSeries(const SeriesData &data, int group);

    QString xLabel;
    QString yLabel;
    QString projectionTitle;

    int tickNumber = 0;
    int option = 0;

    QValueAxis *axisX;
    QValueAxis *axisY;

    QMap<int, SeriesData> pointsByGroup;

protected:
    void mousePressEvent(QMouseEvent *event) override;
};
