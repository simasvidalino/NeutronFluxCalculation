#include "TableInputDlg.h"
#include "ui_TableInputDlg.h"

#include <QClipboard>
#include <regex>

#define prepareChakebox(number) \


TableInputDlg::TableInputDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TableInputDlg)
{
    ui->setupUi(this);

    initDlg();

    setConnections();
}

TableInputDlg::~TableInputDlg()
{
    delete ui;
}

void TableInputDlg::configTable(int row, int column,
                                QStringList &horizontalHeaders,
                                QString &verticalHeader)
{            
    model->configTable(row, column, horizontalHeaders, verticalHeader);

    ui->tableView->setModel(model.get());
}

void TableInputDlg::configTable(int row, QString columnHeader, QString rowHeader)
{
    QStringList header = {columnHeader};
    configTable(row, 1, header, rowHeader);
}

std::vector<double> TableInputDlg::getColumnValues(int column)
{
    QAbstractItemModel *model = ui->tableView->model();
    std::vector<double> values;

    if (!model || column < 0 || column >= model->columnCount()) {
        return values;
    }

    int rowCount = model->rowCount();

    for (int row = 0; row < rowCount; ++row)
    {
        QModelIndex index = model->index(row, column);
        if (index.isValid())
        {
            QVariant value = model->data(index, Qt::EditRole);

            if (value.isNull())
                qWarning() << "The data is null";

            values.push_back(value.toDouble());
        }
    }

    return values;
}

void TableInputDlg::setColumnValues(int column, std::vector<double> values)
{
    auto model = ui->tableView->model();

    if (column < 0)
        return;

    if (column >= model->columnCount())
        model->insertColumn(model->columnCount());

    int rowCount = std::min(model->rowCount(), static_cast<int>(values.size()));

    for (int row = 0; row < rowCount; ++row)
    {
        QModelIndex index = model->index(row, column);
        if (index.isValid())
        {
            model->setData(index, values[row], Qt::EditRole);
        }
    }

}

void TableInputDlg::hideSaveCancelButtons()
{
    ui->buttonBox->hide();
}

void TableInputDlg::initDlg()
{
    model = std::make_unique<CustomTableModel>(this);

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setItemDelegateForColumn(0, new DecimalDelegate(this));

    ui->toolButtonPasteData->setDefaultAction(ui->actionPaste);
}

void TableInputDlg::onPasteFromClipboard()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const char* lineFeedCharacter  = "\n";
    const char* spaceCharacter  = " ";
    bool ok = false;

    QString text = clipboard->text();
    text = text.trimmed();

    auto feedTable = [&](auto list)
    {
        int rowCount = model->rowCount();

        int row = 0;
        for (const auto & value:list){
            if (value.toDouble(&ok)
                    && ok && rowCount > row){
                QModelIndex index = model->index(row, 0);
                model->setData(index, value, Qt::EditRole);
            }
            else{
                qWarning()<<"PasteFromClipboard error: invalid data";
                continue;
            }
            ++row;
        }
    };

    static QRegularExpression delimiterRegExp(R"([\s\n\t\r]+)");
    auto values = text.split(delimiterRegExp, Qt::SkipEmptyParts);

    feedTable(values);

    qInfo()<<"testo colado"<<text;
}

void TableInputDlg::setConnections()
{
    QObject::connect(ui->toolButtonPasteData->defaultAction(), &QAction::triggered, this,
            &TableInputDlg::onPasteFromClipboard);
}
