#include "ChartView.h"
#include <QtCharts/QChart>
#include <QToolTip>
#include <QMouseEvent>
#include <QPen>
#include <QShortcut>
#include <QWheelEvent>
#include <QVBoxLayout>

ChartView::ChartView(QWidget *parent)
    : QChartView{parent},
    axisX(new QValueAxis()),
    axisY(new QValueAxis()),
    legendWindow(new LegendWidget(chart(), nullptr)),
    legendButton(new QToolButton())
{
    init();
}

ChartView::~ChartView()
{
    if (nullptr != legendWindow)
    {
        delete legendWindow;
    }
};

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

    legendButton->setToolTip("Show Legend");
    legendButton->setAutoRaise(true);
    legendButton->setCursor(Qt::PointingHandCursor);
    legendButton->setText("🛈");

    const int btnSize = 28;
    legendButton->setFixedSize(btnSize, btnSize);
    legendButton->setStyleSheet(QString(R"(
    QToolButton {
        background-color: rgba(255, 255, 255, 200);
        border: 1px solid #888;
        border-radius: %1px;
        font-size: 16px;
        font-weight: bold;
        color: #333;
    }
    QToolButton:hover {
        background-color: rgba(240, 240, 240, 240);
        border: 1px solid #444;
    }
    QToolButton:pressed {
        background-color: rgba(220, 220, 220, 255);
    }
)").arg(btnSize / 2));

    legendButtonProxy = chart()->scene()->addWidget(legendButton);
    QRectF plotArea = chart()->plotArea();
    legendButtonProxy->setPos(plotArea.topRight().x() - legendButton->width(), legendButton->height());

    connect(legendButton, &QToolButton::clicked, this, [=]() {
        if (nullptr != legendWindow)
        {
            legendWindow->deleteLater();
            legendWindow = nullptr;
        }

        legendWindow = new LegendWidget(chart());
        legendWindow->show();
    });
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

void ChartView::setRegionLimit(QList<long double> &limit)
{
    this->limit = limit;
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

    setRegionsZonesInChart();
    setRegionLabelsChart();

    if (option == 0)  // All groups
    {
        for (auto it = pointsByGroup.begin(); it != pointsByGroup.end(); ++it)
        {
            addSeries(it.value(), it.key());
        }

        if (pointsByGroup.size() > 20)
        {
            legendButton->show();
            chart()->legend()->hide();
        }
        else
        {
            legendButton->hide();
            chart()->legend()->show();
        }
    }
    else
    {
        legendButton->hide();
        chart()->legend()->show();

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
    serie->setName("G" + QString::number(group + 1));

    QPen pen = serie->pen();
    pen.setColor(data.color);
    pen.setWidth(2);
    serie->setPen(pen);

    chart()->addSeries(serie);
    serie->attachAxis(axisX);
    serie->attachAxis(axisY);
}

void ChartView::clearLabelsChart()
{
    for (QGraphicsItem *item : chart()->scene()->items())
    {
        auto *textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if (textItem && textItem->zValue() == 1000)
        {
            chart()->scene()->removeItem(textItem);
            delete textItem;
        }
    }
}

void ChartView::setRegionLabelsChart()
{
    double yCenter = (axisY->min() + axisY->max()) * 0.5;
    double xBeg    = 0;
    double xEnd    = 0;

    clearLabelsChart();

    for (int i = 0; i < limit.size(); ++i)
    {
        xEnd    = limit[i];
        double xCenter = xBeg + ((xEnd - xBeg) * 0.5);

        auto textItem = new QGraphicsSimpleTextItem(QString("Region %1").arg(i + 1));
        textItem->setBrush(Qt::lightGray);

        QPointF scenePos = chart()->mapToPosition(QPointF(xCenter, yCenter));
        textItem->setPos(scenePos);

        textItem->setZValue(1000);

        chart()->scene()->addItem(textItem);

        xBeg = xEnd;
    }
}

void ChartView::setRegionsZonesInChart()
{
    for (int i = 0; i < limit.size() - 1; ++i)
    {
        auto *boundaryLine = new QLineSeries();
        boundaryLine->append(limit[i], axisY->min());
        boundaryLine->append(limit[i], axisY->max());

        QPen pen(Qt::lightGray);
        pen.setWidth(2);
        pen.setStyle(Qt::DashLine);
        boundaryLine->setPen(pen);

        chart()->addSeries(boundaryLine);
        boundaryLine->attachAxis(axisX);
        boundaryLine->attachAxis(axisY);

        auto markers = chart()->legend()->markers(boundaryLine);
        if (!markers.isEmpty())
            markers.first()->setVisible(false);
    }
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

        setRegionLabelsChart();

        event->accept();
    }
    else
    {
        QChartView::wheelEvent(event);
    }
}

void ChartView::resizeEvent(QResizeEvent *event)
{
    QChartView::resizeEvent(event);

    if (legendButton && legendButtonProxy)
    {
        legendButton->adjustSize();
        QRectF plotArea = chart()->plotArea();

        legendButtonProxy->setPos(plotArea.topRight().x() - legendButton->width(), legendButton->height());
        setRegionLabelsChart();
    }
}
