#include "NJOYInputTableWidget.h"
#include "ui_NJOYInputTableWidget.h"

NJOYInputTableWidget::NJOYInputTableWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NJOYInputTableWidget)
{
    ui->setupUi(this);

    setConnections();

    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->labelExplanation->hide();
}

NJOYInputTableWidget::~NJOYInputTableWidget()
{
    delete ui;
}

void NJOYInputTableWidget::hasItemCountity(bool hasSpinBox)
{
    if (!hasSpinBox)
    {
        ui->spinBox->hide();
        ui->label->hide();
    }
}

void NJOYInputTableWidget::setExplanation(const QString explanation)
{
    ui->labelExplanation->setText(explanation);
    ui->labelExplanation->show();
}

void NJOYInputTableWidget::setTableTitle(QString title)
{
    this->setWindowTitle(title);
}

void NJOYInputTableWidget::setHorizontalHeader(QString str, int column)
{
    auto titleItem = new QTableWidgetItem(str);
    ui->tableWidget->setHorizontalHeaderItem(column, titleItem);
}

QString NJOYInputTableWidget::getColumn(int column)
{
    QString data;

    int row = ui->tableWidget->rowCount();

    for (int i = 0; i < row; ++i)
    {
        auto item = ui->tableWidget->item(i, column);

        if (nullptr != item)
        {
            auto value = item->text();
            data.append(value + ";");
        }
    }

    return data;
}

QStringList NJOYInputTableWidget::getColumnList(int column)
{
    QStringList data;

    int row = ui->tableWidget->rowCount();

    for (int i = 0; i < row; ++i)
    {
        auto item = ui->tableWidget->item(i, column);

        if (nullptr != item)
        {
            auto value = item->text();
            data.append(value);
        }
    }

    return data;
}

void NJOYInputTableWidget::setColumn(const QStringList &newData, int column, int width)
{
    int columnCount = column + 1;
    int rowCount = newData.size();

    if (rowCount  > ui->tableWidget->rowCount())
        ui->tableWidget->setRowCount( rowCount );

    if (columnCount  > ui->tableWidget->columnCount())
        ui->tableWidget->setColumnCount( columnCount );

    for (int index = 0; index < rowCount; ++index)
    {
        auto item = ui->tableWidget->item(index, column);

        if (nullptr == item)
        {
            item =  new QTableWidgetItem(newData.at(index));
            item->setSizeHint(QSize(width, item->sizeHint().height()));
            ui->tableWidget->setItem(index, column, item);

        }
        else
        {
            item->setText( newData.at(index) );
        }
    }

    ui->tableWidget->setColumnWidth(column, width);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(column, QHeaderView::ResizeToContents);

    ui->spinBox->setValue(ui->tableWidget->rowCount());
}

void NJOYInputTableWidget::setColumn(const QString &newData, int column, int width)
{
    auto values = newData.split(";");

    //Treat any wrong input
    values.removeAll("");
    values.removeDuplicates();

    setColumn(values, column, width);
}

void NJOYInputTableWidget::setColumnCount(const int columnCount)
{
    ui->tableWidget->setColumnCount(columnCount);
}

void NJOYInputTableWidget::setRowCount(const int columnCount)
{
    ui->tableWidget->setRowCount(columnCount);

    ui->spinBox->setValue(columnCount);
}

//void NJOYInputTableWidget::setVerticalHeader(const QList<QString> headers)
//{
//    ui->tableWidget->setRowCount(headers.count());

//    int row = 0;

//    for (const auto &header : headers)
//    {
//        QTableWidgetItem *item = new  QTableWidgetItem(header);
//        ui->tableWidget->setVerticalHeaderItem(row, item);
//        ++row;
//    }
//}

int NJOYInputTableWidget::getSelectedRow() const
{
    return ui->tableWidget->currentRow();
}

void NJOYInputTableWidget::setCurrentRow(int nRow)
{
    ui->tableWidget->setCurrentCell(nRow, 0);
}

QString NJOYInputTableWidget::getSelectedText(int column)
{
    QString text;

    auto items = ui->tableWidget->selectedItems();

    for (int index = 0; index < items.size(); ++index)
    {
        if (items[index]->column() == column)
            text.push_back(items[index]->text() + ";");
    }

    return text;
}

void NJOYInputTableWidget::setDataRange(const int min, const int max)
{
    ui->spinBox->setRange(min, max);
}

void NJOYInputTableWidget::changeRowNumber(int rowCount)
{
    ui->tableWidget->setRowCount(rowCount);
    const int columnCount = ui->tableWidget->columnCount();

    for (int iIndex = 0; iIndex < rowCount; ++iIndex)
    {
        for (int jIndex = 0; jIndex < columnCount; ++jIndex)
        {
            auto item = ui->tableWidget->item(rowCount, columnCount);

            if (nullptr == item)
            {
                ui->tableWidget->setItem(iIndex, jIndex, new QTableWidgetItem());
            }

        }

    }
}

void NJOYInputTableWidget::setConnections()
{
    connect(ui->spinBox, &QSpinBox::valueChanged, this, &NJOYInputTableWidget::changeRowNumber);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

