#include "ChartView.h"
#include "qlineedit.h"
#include "qtimer.h"

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
    QTimer::singleShot(1000, this, [&](){
        init();
    });
}

ChartView::~ChartView()
{

}

void ChartView::setInputData(QList<QPointF> &value, int group)
{
    auto it = seriesByGroup.find(group);

    if (it != seriesByGroup.end())
    {
        delete it->second;
    }

    QLineSeries *serie = new QLineSeries();
    for (const auto &point : value)
    {
        serie->append(point);
    }

    serie->attachAxis(axisY);
    serie->attachAxis(axisX);

    seriesByGroup[group] = serie;

    serie->setName("Group " + QString::number(group + 1));
}

void ChartView::setInputData(QList<QPointF> &value)
{
    auto it = seriesByGroup.find(0);
    if (it != seriesByGroup.end())
    {
        delete it->second;
    }

    QLineSeries *serie = new QLineSeries();

    for (const auto &point : value)
        serie->append(point);

    serie->attachAxis(axisY);
    serie->attachAxis(axisX);

    seriesByGroup[0] = serie;
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

    periodicity->setRange(1, max);
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
        options << "Group " + QString::number(group + 1);
        ++group;
    }

    //if we have only one group we don't need a filter
    if (group == 1)
        filter->hide();
    else
        filter->show();

    filter->setFixedWidth(300);

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
    this->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setRenderHint(QPainter::Antialiasing);

    axisX->setLabelFormat("%.2f");
    axisY->setLabelFormat("%.2f");

    auto mChart = new QChart();
    mChart->legend()->setAlignment(Qt::AlignTop);
    mChart->addAxis(axisX, Qt::AlignBottom);
    mChart->addAxis(axisY, Qt::AlignLeft);

    QChartView::setChart(mChart);

    this->setRenderHint(QPainter::Antialiasing);

    int widgetLeft = 10;
    filter = new CustomComboBox();
    filter->move(widgetLeft, 10);


    periodicityLabel = new QLabel("Periodicity:", this);
    periodicityLabel->move(widgetLeft, filter->geometry().height() + 1);

    periodicity = new QSpinBox(this);
    periodicity->move(periodicityLabel->geometry().right(), filter->geometry().height() + 1);
    periodicity->setSingleStep(5);
    periodicity->setSuffix("cm");
    periodicity->setButtonSymbols(QAbstractSpinBox::NoButtons);
    //periodicity->setDecimals(1);
    periodicity->setValue(15);

    QLineEdit *lineEdit = periodicity->findChild<QLineEdit*>(); //@TBDprotected member, the right way is create a child class
    lineEdit->setFrame(false);
    periodicity->setFrame(false);

    //We will start with hidden widgets
    periodicityLabel->hide();
    periodicity->hide();
    filter->hide();

    setConnection();
}

void ChartView::setConnection()
{
    //connect(filter, &QComboBox::activated, this, &ChartView::filterChange);
    connect(periodicity, &QSpinBox::valueChanged, this, [this](auto value)
    {
        emit updatePeriodicity(value);}
    );
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (false == this->hasFocus())
        return;

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

void ChartView::hideLegend()
{
    this->chart()->legend()->hide();
}

CustomComboBox::CustomComboBox(QWidget *parent) : QComboBox(parent)
{    
    qInfo() << "CustomComboBox constructor";

    this->setFixedWidth(300);
    this->setFrame(false);
    this->setStyleSheet(R"(
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

}

void CustomComboBox::focusOutEvent(QFocusEvent *event)
{
    qInfo()<<"focusOutEvent";
    this->clearFocus();

    QWidget * parent = nullptr;
    parent = this->parentWidget();

    if (nullptr != parent)
        parent->setFocus();

    QComboBox::focusOutEvent(event);
}

void CustomComboBox::leaveEvent(QEvent *event)
{
    qInfo()<<"leaveEvent";
    this->clearFocus();

    QWidget * parent = nullptr;
    parent = this->parentWidget();

    if (nullptr != parent)
        parent->setFocus();

    QComboBox::leaveEvent(event);
}
