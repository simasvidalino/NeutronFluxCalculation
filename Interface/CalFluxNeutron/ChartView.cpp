#include "ChartView.h"

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

ChartView::ChartView(QWidget *parent)
    : QChartView{parent},
      chart(new QChart()),
      axisX(new QValueAxis()),
      axisY(new QValueAxis())
{
    axisX->setLabelFormat("%.2f");
    axisY->setLabelFormat("%.2f");
    chart->legend()->hide();

    this->setRenderHint(QPainter::Antialiasing);
}

ChartView::~ChartView()
{
}

void ChartView::setInputData(QList<QPointF> &value, QList<int> &regions, int group)
{
    QLineSeries *serie = new QLineSeries();
    regionSize = regions;

    for (const auto &point : value)
        serie->append(point);

    seriesByGroup[group] = serie;
}

void ChartView::setXLabel(const QString &name)
{
    xLabel = name;
    axisX->setTitleText(xLabel);
}

void ChartView::setYLabel(const QString &name)
{
    yLabel = name;
    axisY->setTitleText(yLabel);
}
void ChartView::setProjectionTitle(const QString value)
{
    projectionTitle = value;
    chart->setTitle(projectionTitle);
}

void ChartView::setTickNumber(int newTickNumber)
{
    tickNumber = newTickNumber;
    axisX->setTickCount(tickNumber);
}

void ChartView::setChart()
{
    for (const auto &pair : seriesByGroup)
    {
        QLineSeries *serie = pair.second;

        if (serie && chart)
        {
            chart->addSeries(serie);

            chart->addAxis(axisX, Qt::AlignBottom);
            serie->attachAxis(axisX);

            chart->addAxis(axisY, Qt::AlignLeft);
            serie->attachAxis(axisY);
        }
        else
        {
            qWarning() << "Serie is Null";
        }
    }

    QChartView::setChart(chart);
}

void ChartView::clearChart()
{
    if (!chart->axes().isEmpty())
    {
        chart->removeAllSeries();

        seriesByGroup.clear();
    }
}
