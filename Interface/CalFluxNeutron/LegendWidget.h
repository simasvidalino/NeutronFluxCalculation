#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QDialog>
#include <QtCharts>

class LegendWidget : public QDialog
{
    Q_OBJECT
public:
    explicit LegendWidget(QChart *chart, QWidget *parent = nullptr);
    virtual ~LegendWidget();

private:
    void setupLegend(QChart *chart);
};

#endif // LEGENDWIDGET_H
