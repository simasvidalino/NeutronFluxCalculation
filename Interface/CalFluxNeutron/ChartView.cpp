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

    series = std::make_unique<QLineSeries>();
}

ChartView::~ChartView()
{

}

void ChartView::setInputData(double &maxY, std::unique_ptr<QLineSeries> &&value)
{
    series = std::move(value);

    this->maxY = maxY;
}

QChart *ChartView::getChart()
{
    return chart.get();
}

void ChartView::setProjectionTitle(const QString value)
{
    projectionTitle = value;
}

void ChartView::setGradesLabel(const QString &name)
{
    gradeLabel = name;
}

void ChartView::setChart()
{
    if (!chart->axes().isEmpty())
        clearChart();

    chart->legend()->hide();
    chart->setTitle(projectionTitle);
    chart->addSeries(series.get());

    setAxes();

    QChartView::setChart(chart.get());
}

void ChartView::clearChart()
{
    chart->removeSeries(series.get());
    chart->removeAxis(axisX.get());
    chart->removeAxis(axisY.get());
}

void ChartView::setAxes()
{
    axisX->setTitleText("Position x (cm)");
    axisX->setLabelFormat("%i");

    int tickNumber = 12;
    axisX->setTickCount(tickNumber);
    chart->addAxis(axisX.get(), Qt::AlignBottom);

    series->attachAxis(axisX.get());

    axisY->setTitleText(gradeLabel);
    axisY->setLabelFormat("%.2f");
    axisY->setRange(0, maxY + 2);
    chart->addAxis(axisY.get(), Qt::AlignLeft);

    series->attachAxis(axisY.get());
}




