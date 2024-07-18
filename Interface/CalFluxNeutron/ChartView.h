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

class CustomComboBox : public QComboBox {
    Q_OBJECT

public:
    using QComboBox::QComboBox;

    explicit CustomComboBox(QWidget* parent = nullptr);

protected:
    virtual void focusOutEvent(QFocusEvent *event) override;

    // QWidget interface
protected:
    virtual void leaveEvent(QEvent *event) override;
};

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

    int getPeriodicity();
    void setPeriodicity(int value);

    void setTickNumber(int newTickNumber);

    void setYRange(int min, int max);

    void setXRange(int min, int max);

    void setFilterByGroup();

    void showPeriodicity();

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

    double maxY = 1;
    int tickNumber;

    QValueAxis* axisX;
    QValueAxis* axisY;

    std::map<int, QLineSeries*> seriesByGroup;

    QList<int> regionSize;

    QList<QColor> colors = {Qt::blue, Qt::red, Qt::cyan};

    QComboBox* filter = nullptr;
    QSpinBox* periodicity = nullptr;
    QLabel* periodicityLabel = nullptr;

    int option = 0;

    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
};

