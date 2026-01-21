
#include "NeutronAnalysisChartWidget.h"
#include "ui_NeutronAnalysisChartWidget.h"

#include <QLineEdit>

#include "InterFaceDefinitions.h"

NeutronAnalysisChartWidget::NeutronAnalysisChartWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NeutronAnalysisChartWidget)
    , totalRegionSize(0.0)
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
    ui->widgetChart->blockSignals(true);
    ui->widgetChart->filterChange(0); // Always reset to 'All'
    ui->widgetChart->blockSignals(false);

    addTableItemsByGroup();
    addTableItemsByRegion();

    styleTable();
    ui->tableWidget->setCurrentCell(ui->tableWidget->rowCount() - 1, 0);
}

int NeutronAnalysisChartWidget::getPeriodicityValue() const
{
    return ui->spinBoxPeriodicity->value();
}

void NeutronAnalysisChartWidget::setMaxPeriodicity(double periodicityMaxValue)
{
    ui->spinBoxPeriodicity->setMaximum(periodicityMaxValue);
}

void NeutronAnalysisChartWidget::setPeriodicityValue(double periodicityValue)
{
    ui->spinBoxPeriodicity->setValue(periodicityValue);
}

void NeutronAnalysisChartWidget::setProjectionTitle(const QString &value)
{
    ui->widgetChart->setProjectionTitle(value);
}

void NeutronAnalysisChartWidget::setRange(long double x1, long double y1, long double x2, long double y2)
{
    ui->widgetChart->setXRange(std::floor(x1), std::ceil(x2));
    ui->widgetChart->setYRange(std::floor(y1), std::ceil(y2));

    totalRegionSize = x2;
}

void NeutronAnalysisChartWidget::setRegionLimit(QList<long double> &limit)
{
    ui->widgetChart->setRegionLimit(limit);
}

void NeutronAnalysisChartWidget::setZonesNames(QStringList& zone)
{
    ui->widgetChart->setZonesNames(zone);
}

void NeutronAnalysisChartWidget::setTableItemsByGroup(std::vector<std::vector<long double> > &item)
{
    tableItemByGroup = item;
}

void NeutronAnalysisChartWidget::setTotalByRegion(std::vector<long double> &item)
{
    tableItemByRegion = item;
}

void NeutronAnalysisChartWidget::setTableDimension(int rowCount, int columnCount)
{
    //We don't need "Total" row because "Total is the sum of all groups
    if (rowCount == 2)
        rowCount = 1;

    ui->tableWidget->setRowCount(rowCount);
    ui->tableWidget->setColumnCount(columnCount);
}

void NeutronAnalysisChartWidget::setTableHeaders(QStringList &horizontalHeaderStr, QStringList &verticalHeaderStr)
{
    ui->tableWidget->setHorizontalHeaderLabels(horizontalHeaderStr);
    ui->tableWidget->setVerticalHeaderLabels(verticalHeaderStr);
}

void NeutronAnalysisChartWidget::setTableTitle(const QString &title)
{
    ui->labelTableTitle->setText(title);
}

void NeutronAnalysisChartWidget::addTableItemsByGroup()
{
    if (tableItemByGroup.empty())
        return;

    int rowCount    = tableItemByGroup[0].size();
    int columnCount = tableItemByGroup.size();
    int precision   = Interface::getPrecision();

    for (int row = 0; row < rowCount; ++row)
    {
        for (int col = 0; col < columnCount; ++col)
        {
            const QString value           = QString::number(static_cast<double>(tableItemByGroup[col][row]), 'g', precision);
            QTableWidgetItem *measureItem = new QTableWidgetItem(value);
            measureItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(row, col, measureItem);
        }
    }
}

void NeutronAnalysisChartWidget::addTableItemsByRegion()
{
    if (tableItemByRegion.empty())
        return;

    int rowCount    = ui->tableWidget->rowCount();
    int columnCount = tableItemByRegion.size();
    int precision   = Interface::getPrecision();

    for (int col = 0; col < columnCount; ++col)
    {
        const QString value           = QString::number(static_cast<double>(tableItemByRegion[col]), 'g', precision);
        QTableWidgetItem *measureItem = new QTableWidgetItem(value);
        measureItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(rowCount - 1, col, measureItem);
    }
}

void NeutronAnalysisChartWidget::init()
{
    setConnections();

    ui->labelPeriodicity->setVisible(false);
    ui->spinBoxPeriodicity->setVisible(false);

    QLineEdit *lineEdit = ui->spinBoxPeriodicity->findChild<QLineEdit *>();
    lineEdit->setFrame(false);

    ui->comboBoxFilterGroup->setFixedWidth(300);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void NeutronAnalysisChartWidget::setConnections()
{
    QObject::connect(ui->spinBoxPeriodicity, &QSpinBox::valueChanged, this, &NeutronAnalysisChartWidget::updateChartStep);

    QObject::connect(ui->comboBoxFilterGroup, &QComboBox::currentIndexChanged, this, [this](int index)
                     {
                         ui->widgetChart->filterChange(index);
                         if (index != 0) // Not "All"
                         {
                             ui->tableWidget->setCurrentCell(index - 1, 0);
                         }
                         else
                         {
                             ui->tableWidget->setCurrentCell(ui->tableWidget->rowCount() - 1, 0);
                         }
                     });
}

void NeutronAnalysisChartWidget::styleTable()
{
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void NeutronAnalysisChartWidget::updateChartStep(int step)
{
    int tickCount = totalRegionSize / step;
    ui->widgetChart->setTickNumber(tickCount + 1);
}

void NeutronAnalysisChartWidget::addSeries(QList<QPointF> &value, int group)
{
    ui->widgetChart->setInputData(value, group);
}

void NeutronAnalysisChartWidget::setFilteredByGroup(int group)
{
    bool hasOneGroup = (group == 1);

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
