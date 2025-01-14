#ifndef NEUTRONANALYSISCHARTWIDGET_H
#define NEUTRONANALYSISCHARTWIDGET_H

#include <QWidget>

namespace Ui
{
class NeutronAnalysisChartWidget;
}

class NeutronAnalysisChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NeutronAnalysisChartWidget(QWidget *parent = nullptr);
    virtual ~NeutronAnalysisChartWidget();

    void addSeries(QList<QPointF> &value, int group);

    void clearChart();

    void commitChanges();

    int getPeriodicityValue() const;

    void setFilteredByGroup(int group);

    void setLabels(const QString &xLabel, const QString &yLabel);

    void setMaxPeriodicity(double periodicity);

    void setPeriodicityValue(double periodicity);

    void setProjectionTitle(const QString &value);

    void setRange(long double x1 = 0.0, long double y1 = 0.0 ,
                  long double x2 = 0.0, long double y2 = 0.0);

    void setTableItems(std::vector<std::vector<long double> > &&item);

    void setTableDimension(int rowCount, int columnCount);

    void setTableHeaders(QStringList &horizontalHeaderStr, QStringList &verticalHeaderStr);

private:
    Ui::NeutronAnalysisChartWidget *ui;

    void addTableItems();

    void init();

    void setConnections();

    void updateChartStep(int step);

    std::vector<std::vector<long double>> tableItem;
    double totalRegionSize;
};

#endif
