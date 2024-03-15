#include "ChartView.h"
#include "qgraphicsproxywidget.h"
#include "qlineedit.h"

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QToolTip>

ChartView::ChartView(QWidget *parent)
    : QChartView{parent},
      axisX(new QValueAxis()),
      axisY(new QValueAxis()),
      filter(nullptr)
{
    init();

    setConnection();
}

ChartView::~ChartView()
{

}

void ChartView::setInputData(QList<QPointF> &value, int group)
{
    auto it = seriesByGroup.find(group);
    if (it != seriesByGroup.end())
        delete it->second;

    QLineSeries *serie = new QLineSeries();
    for (const auto &point : value)
        serie->append(point);

    serie->attachAxis(axisY);
    serie->attachAxis(axisX);

    seriesByGroup[group] = serie;

    serie->setName("Group " + QString::number(group + 1));
}
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
    this->chart()->setTitle(projectionTitle);
}

int ChartView::getPeriodicity()
{
    return periodicity->value();
}

void ChartView::setPeriodicity(int value)
{
    periodicity->setValue(value);
}

void ChartView::setTickNumber(int newTickNumber)
{
    tickNumber = newTickNumber;
    axisX->setTickCount(tickNumber);
}

void ChartView::setYRange(int min, int max)
{
    axisY->setRange(min, max);
}

void ChartView::setXRange(int min, int max)
{
    maxY = max;
    axisX->setRange(min, max);
}

void ChartView::setFilterByGroup()
{
    if (seriesByGroup.empty())
        return;

    int group = 0;
    QStringList options;
    options << "All";

    for (const auto &pair : seriesByGroup)
    {
        options << "Group " + QString::number(group);
        ++group;
    }

    //if we have only one group we don't need a filter
    if (group == 1)
        filter->hide();
    else
        filter->show();

    filter->clear();
    filter->addItems(options);
}

void ChartView::showPeriodicity()
{
    periodicityLabel->show();
    periodicity->show();
}

void ChartView::setChart()
{
    if (seriesByGroup.empty())
        return;

    //clear
    QList<QAbstractSeries *> allSeries = chart()->series();
    for (QAbstractSeries *series : allSeries)
    {
        chart()->removeSeries(series);
    }

    auto addSerie = [&](QLineSeries *serie)
    {
        if (serie && this->chart())
        {
            this->chart()->addSeries(serie);

            if (!serie->attachAxis(axisX))
                serie->attachAxis(axisX);

            if (!serie->attachAxis(axisY))
                serie->attachAxis(axisY);
        }
        else
        {
            qWarning() << "Serie is Null";
        }
    };

    if (option == 0) //all
    {
        for (const auto &pair : seriesByGroup)
        {
            QLineSeries *serie = pair.second;

            addSerie(serie);
        }
    }
    else
    {
        int iIndex = option - 1;

        if (iIndex < seriesByGroup.size()
                && iIndex >= 0)
        {
            auto serie = seriesByGroup[iIndex];

            addSerie(serie);
        }
    }
}

void ChartView::filterChange(int option)
{
    this->option = option;
    setChart();
}

void ChartView::init()
{
    axisX->setLabelFormat("%.2f");
    axisY->setLabelFormat("%.2f");

    auto mChart = new QChart();
    mChart->legend()->setAlignment(Qt::AlignTop);
    mChart->addAxis(axisX, Qt::AlignBottom);
    mChart->addAxis(axisY, Qt::AlignLeft);

    QChartView::setChart(mChart);

    this->setRenderHint(QPainter::Antialiasing);

    int widgetLeft = 10;
    filter = new QComboBox(this);
    filter->move(widgetLeft, 10);
    filter->setFrame(false);
    filter->setStyleSheet(R"(
        QComboBox {
            border: none;
            padding: 1px 18px 1px 3px;
        }
        QComboBox::drop-down {
            width: 0px;
        }
        QComboBox::down-arrow {
            image: none;
        }
    )");


    periodicityLabel = new QLabel("Periodicity:", this);
    periodicityLabel->move(widgetLeft, filter->geometry().height() + 1);

    periodicity = new QSpinBox(this);
    periodicity->move(periodicityLabel->geometry().right(), filter->geometry().height() + 1);
    periodicity->setSingleStep(5);
    periodicity->setSuffix("cm");
    periodicity->setRange(5, 200);
    periodicity->setButtonSymbols(QAbstractSpinBox::NoButtons);
    periodicity->setValue(15);

    QLineEdit *lineEdit = periodicity->findChild<QLineEdit*>(); //@TBDprotected member, the right way is create a child class
    lineEdit->setFrame(false);
    periodicity->setFrame(false);

    QGraphicsProxyWidget *proxyWidgetFilter = new QGraphicsProxyWidget;
    proxyWidgetFilter->setWidget(filter);
    QGraphicsProxyWidget *proxyWidgetPeriodicity = new QGraphicsProxyWidget;
    proxyWidgetPeriodicity->setWidget(filter);
    QGraphicsProxyWidget *proxyWidgetPeriodicityLabel = new QGraphicsProxyWidget;
    proxyWidgetPeriodicityLabel->setWidget(filter);

    this->scene()->addItem(proxyWidgetFilter);
    this->scene()->addItem(proxyWidgetPeriodicity);
    this->scene()->addItem(proxyWidgetPeriodicityLabel);

    //We will start with hidden widgets
    periodicityLabel->hide();
    periodicity->hide();
    filter->hide();
}

void ChartView::setConnection()
{
    connect(filter, &QComboBox::activated, this, &ChartView::filterChange);
    connect(periodicity, &QSpinBox::valueChanged, this, [this](int value){
        emit updatePeriodicity(value);}
    );
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    QPointF cursorPoint = chart()->mapToValue(event->pos());

    bool pointFound = false;
    QString tooltipText;
    const double proximityThreshold = 1.0;

    for (auto series : chart()->series())
    {
        auto lineSeries = dynamic_cast<QLineSeries*>(series);
        if (lineSeries)
        {
            for (const QPointF &point : lineSeries->points())
            {
                double distance = QLineF(point, cursorPoint).length();
                if (distance < proximityThreshold)
                {
                    tooltipText = QString("X: %1, Y: %2").arg(point.x()).arg(point.y());
                    pointFound = true;
                    break;
                }
            }
        }
        if (pointFound)
        {
            break;
        }
    }

    if (pointFound)
    {
        QToolTip::showText(event->globalPosition().toPoint(), tooltipText, this, QRect(), 10000);
    }
    else
    {
        QToolTip::hideText();
    }

    QChartView::mouseMoveEvent(event);
}

void ChartView::clearChart()
{
    if (!this->chart()->axes().isEmpty())
    {
        this->chart()->removeAllSeries();

        seriesByGroup.clear();
    }
}
