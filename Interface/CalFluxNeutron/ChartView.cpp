#include "ChartView.h"

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

ChartView::ChartView(QWidget *parent)
    : QChartView{parent}
{
    chart = std::make_unique<QChart>();
    axisX = std::make_unique<QValueAxis>();
    axisY = std::make_unique<QValueAxis>();

    axisX->setLabelFormat("%.2f");
    axisY->setLabelFormat("%.2f");
    chart->legend()->hide();
}

ChartView::~ChartView()
{

}

void ChartView::setInputData(QList<QPointF> &value, int group)
{
    auto serie = std::make_unique<QLineSeries>();

    for (const auto &point : value)
        serie->append(point);

    auto pair = std::make_pair(group, std::move(serie));
    seriesByGroup.emplace(std::move(pair));
}

QChart *ChartView::getChart()
{
    return chart.get();
}

void ChartView::setXLabel(const QString &name)
{
    xLabel = name;
    axisX->setTitleText(xLabel);
}

void ChartView::setProjectionTitle(const QString value)
{
    projectionTitle = value;
    chart->setTitle(projectionTitle);
}

void ChartView::setYLabel(const QString &name)
{
    yLabel = name;
    axisY->setTitleText(yLabel);
}

void ChartView::setChart()
{
    if (!chart->axes().isEmpty())
        clearChart();

    for (const auto &pair : seriesByGroup)
    {
        auto serie = pair.second.get();

        qInfo()<<serie;
        chart->addSeries(serie);

        chart->addAxis(axisX.get(), Qt::AlignBottom);
        serie->attachAxis(axisX.get());

        axisY->setRange(0, maxY);
        chart->addAxis(axisY.get(), Qt::AlignLeft);

        serie->attachAxis(axisY.get());
    }

    QChartView::setChart(chart.get());
}

void ChartView::clearChart()
{
    //chart->removeSeries(series.get());
    chart->removeAxis(axisX.get());
    chart->removeAxis(axisY.get());
    //chart = std::make_unique<QChart>();
}

void ChartView::setAxes()
{
//    chart->addAxis(axisX.get(), Qt::AlignBottom);

//    series->attachAxis(axisX.get());

//    axisY->setRange(0, maxY);
//    chart->addAxis(axisY.get(), Qt::AlignLeft);

//    series->attachAxis(axisY.get());
}

void ChartView::setTickNumber(int newTickNumber)
{
    tickNumber = newTickNumber;
    axisX->setTickCount(tickNumber);
}




