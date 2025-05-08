#pragma once

#include <QChartView>
#include <QLineSeries>
#include <QMap>
#include <QValueAxis>

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

    void setInputData(const QList<QPointF> &points, int group, QColor color = QColor());

    void setXLabel(const QString &label);
    void setYLabel(const QString &label);
    void setProjectionTitle(const QString &title);

    void setTickNumber(int newTickNumber);
    void setYRange(double min, double max);
    void setXRange(double min, double max);

    void clearChart();
    void clearData();
    void hideLegend();

public slots:
    void setChart();
    void filterChange(int option);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void init();
    void addSeries(const SeriesData &data, int group);

    QString xLabel;
    QString yLabel;
    QString projectionTitle;

    QValueAxis *axisX;
    QValueAxis *axisY;
    int tickNumber = 0;
    int option     = 0;

    QMap<int, SeriesData> pointsByGroup;
};
