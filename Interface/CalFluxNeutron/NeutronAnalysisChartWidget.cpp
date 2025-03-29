#include "NeutronAnalysisChartWidget.h"
#include "ui_NeutronAnalysisChartWidget.h"

#include <QLineEdit>

NeutronAnalysisChartWidget::NeutronAnalysisChartWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NeutronAnalysisChartWidget),
    totalRegionSize(0.0)
{
    ui->setupUi(this);

    init();
}

NeutronAnalysisChartWidget::~NeutronAnalysisChartWidget()
{
    delete ui;
}

void NeutronAnalysisChartWidget::clearChart()
{
    ui->widgetChart->clearChart();
}

void NeutronAnalysisChartWidget::clearTable()
{
    ui->tableWidget->clear();
}

void NeutronAnalysisChartWidget::commitChanges()
{
    ui->widgetChart->filterChange(0); //Set "All" option in filter

    ui->widgetChart->setChart();

    addTableItems();
}

int NeutronAnalysisChartWidget::getPeriodicityValue() const
{
    return ui->spinBoxPeriodicity->value();
}

void NeutronAnalysisChartWidget::setMaxPeriodicity(double periodicity)
{
    ui->spinBoxPeriodicity->setMaximum(periodicity);
}

void NeutronAnalysisChartWidget::setPeriodicityValue(double periodicity)
{
    ui->spinBoxPeriodicity->setMaximum(periodicity);
}

void NeutronAnalysisChartWidget::setProjectionTitle(const QString &value)
{
    ui->widgetChart->setProjectionTitle(value);
}

void NeutronAnalysisChartWidget::setRange(long double x1, long double y1,
                                          long double x2, long double y2)
{
    ui->widgetChart->setXRange(std::floor(x1), std::ceil(x2));
    ui->widgetChart->setYRange(std::floor(y1), std::ceil(y2));

    totalRegionSize = x2;
}

void NeutronAnalysisChartWidget::setTableItems(std::vector<std::vector<long double> > &&item)
{
    tableItem = std::move(item); ;
}

void NeutronAnalysisChartWidget::setTableDimension(int rowCount, int columnCount)
{
    ui->tableWidget->setRowCount(rowCount);
    ui->tableWidget->setColumnCount(columnCount);
}

void NeutronAnalysisChartWidget::setTableHeaders(QStringList &horizontalHeaderStr, QStringList &verticalHeaderStr)
{
    ui->tableWidget->setHorizontalHeaderLabels(horizontalHeaderStr);
    ui->tableWidget->setVerticalHeaderLabels(verticalHeaderStr);
}

void NeutronAnalysisChartWidget::addTableItems()
{
    if (tableItem.empty())
        return;

    const auto rowCount    = ui->tableWidget->rowCount();
    const auto columnCount = ui->tableWidget->columnCount();

    for (int col = 0; col < columnCount; ++col)
    {
        for (int row = 0; row < rowCount; ++row)
        {
            QTableWidgetItem *measureItem = new QTableWidgetItem(QString::number( static_cast<double>( tableItem[col][row]) ));
            measureItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(row, col, measureItem);
        }
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void NeutronAnalysisChartWidget::init()
{
    setConnections();

    QLineEdit *lineEdit = ui->spinBoxPeriodicity->findChild<QLineEdit*>(); //@TBDprotected member, the right way is create a child class
    lineEdit->setFrame(false);

    ui->comboBoxFilterGroup->setFixedWidth(300);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void NeutronAnalysisChartWidget::setConnections()
{
    connect(ui->spinBoxPeriodicity, &QSpinBox::valueChanged, this, &NeutronAnalysisChartWidget::updateChartStep);


    connect(ui->comboBoxFilterGroup, &QComboBox::currentIndexChanged, this, [this](int index)
            {
                ui->widgetChart->filterChange(index);

                if (index != 0) //all
                {
                    ui->tableWidget->setCurrentCell(index - 1, 0);
                }
            });
}

void NeutronAnalysisChartWidget::updateChartStep(int step)
{
    int tickCount = totalRegionSize/step;

    ui->widgetChart->setTickNumber(tickCount + 1);
    ui->widgetChart->setChart();
}

void NeutronAnalysisChartWidget::addSeries(QList<QPointF> &value, int group)
{
    ui->widgetChart->setInputData(value, group);
}

void NeutronAnalysisChartWidget::setFilteredByGroup(int group)
{
    //if we have only one group we don't need a filter
    bool hasOneGroup = ( group == 1 );

    if (hasOneGroup)
    {
        ui->comboBoxFilterGroup->hide();
        ui->labeFilterGroup->hide();

        return;
    }

    QStringList options;
    options << "All";

    for (int ig = 0; ig < group; ++ig)
    {
        options << "Group " + QString::number(ig + 1);
    }

    ui->comboBoxFilterGroup->blockSignals(true);
    ui->comboBoxFilterGroup->show();
    ui->labeFilterGroup->show();
    ui->comboBoxFilterGroup->clear();
    ui->comboBoxFilterGroup->addItems(options);
    ui->comboBoxFilterGroup->blockSignals(false);

}

void NeutronAnalysisChartWidget::setLabels(const QString &xLabel, const QString &yLabel)
{
    ui->widgetChart->setXLabel(xLabel);
    ui->widgetChart->setYLabel(yLabel);
}
