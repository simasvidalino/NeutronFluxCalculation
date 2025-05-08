#include "ChartView.h"
#include <QtCharts/QChart>
#include <QToolTip>
#include <QMouseEvent>
#include <QPen>
#include <QShortcut>
#include <QWheelEvent>

ChartView::ChartView(QWidget *parent)
    : QChartView{parent},
    axisX(new QValueAxis()),
    axisY(new QValueAxis())
{
    init();
}

ChartView::~ChartView() = default;

void ChartView::init()
{
    setRubberBand(QChartView::RectangleRubberBand);
    setDragMode(QGraphicsView::ScrollHandDrag);

    QShortcut *resetShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z), this);
    QObject::connect(resetShortcut, &QShortcut::activated, this, [this]() {
        chart()->zoomReset();
    });

    this->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    this->setRenderHint(QPainter::Antialiasing);

    axisX->setLabelFormat("%.2f");
    axisY->setLabelFormat("%.2f");

    auto *mChart = new QChart();
    mChart->legend()->setAlignment(Qt::AlignTop);
    mChart->addAxis(axisX, Qt::AlignBottom);
    mChart->addAxis(axisY, Qt::AlignLeft);

    mChart->setPlotAreaBackgroundVisible(true);

    QChartView::setChart(mChart);
}

void ChartView::setInputData(const QList<QPointF> &points, int group, QColor color)
{
    SeriesData data;
    data.points = points;

    if (!color.isValid())
    {
        int hue = (group * 137) % 360; // 137 is prime number for better distribution
        int saturation = 160 + (group * 73) % 96;
        int value = 160 + (group * 199) % 96;

        data.color = QColor::fromHsv(hue, saturation, value);

        if (!data.color.isValid())
        {
            data.color = Qt::red;
        }
    }
    else
    {
        data.color = color;
    }

    pointsByGroup[group] = data;
}

void ChartView::setXLabel(const QString &label)
{
    xLabel = label;
    axisX->setTitleText(label);
}

void ChartView::setYLabel(const QString &label)
{
    yLabel = label;
    axisY->setTitleText(label);
}

void ChartView::setProjectionTitle(const QString &title)
{
    projectionTitle = title;
    chart()->setTitle(title);
}

void ChartView::setTickNumber(int newTickNumber)
{
    tickNumber = newTickNumber;
    axisX->setTickCount(tickNumber);
}

void ChartView::setYRange(double min, double max)
{
    axisY->setRange(min, max);
}

void ChartView::setXRange(double min, double max)
{
    axisX->setRange(min, max);
}

void ChartView::setChart()
{
    if (pointsByGroup.isEmpty())
        return;

    chart()->zoomReset();
    chart()->removeAllSeries();

    if (option == 0)  // All groups
    {
        for (auto it = pointsByGroup.begin(); it != pointsByGroup.end(); ++it)
        {
            addSeries(it.value(), it.key());
        }
    }
    else
    {
        int group = option - 1;
        if (pointsByGroup.contains(group))
        {
            addSeries(pointsByGroup.value(group), group);
        }
    }
}

void ChartView::filterChange(int option)
{
    this->option = option;
    setChart();
}

void ChartView::addSeries(const SeriesData &data, int group)
{
    if (data.points.isEmpty())
        return;

    auto *serie = new QLineSeries();
    serie->append(data.points);
    serie->setName("Group " + QString::number(group + 1));

    QPen pen = serie->pen();
    pen.setColor(data.color);
    pen.setWidth(2);
    serie->setPen(pen);

    chart()->addSeries(serie);
    serie->attachAxis(axisX);
    serie->attachAxis(axisY);
}

void ChartView::clearChart()
{
    chart()->removeAllSeries();
    pointsByGroup.clear();
}

void ChartView::clearData()
{
    clearChart();
}

void ChartView::hideLegend()
{
    chart()->legend()->hide();
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (!hasFocus())
        return;

    QPointF cursorPoint = chart()->mapToValue(event->pos());
    QString tooltipText;
    const double proximityThreshold = 1.0;

    for (auto *series : chart()->series())
    {
        auto *lineSeries = qobject_cast<QLineSeries *>(series);
        if (lineSeries)
        {
            for (const QPointF &point : lineSeries->points())
            {
                if (QLineF(point, cursorPoint).length() < proximityThreshold)
                {
                    tooltipText = QString("X: %1, Y: %2").arg(point.x()).arg(point.y());
                    QToolTip::showText(event->globalPosition().toPoint(), tooltipText, this, QRect(), 10000);
                    return;
                }
            }
        }
    }

    QToolTip::hideText();
    QChartView::mousePressEvent(event);
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->angleDelta().y() > 0)
        {
            chart()->zoomIn();
        }
        else
        {
            chart()->zoomOut();
        }
        event->accept();
    }
    else
    {
        QChartView::wheelEvent(event);
    }
}
