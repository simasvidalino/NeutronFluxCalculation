#include "TableInputDlg.h"
#include "ui_TableInputDlg.h"

#include <QClipboard>

TableInputDlg::TableInputDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TableInputDlg),
    undoStack(new QUndoStack(this))
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
    model = std::make_unique<GradesTableModel>(this);

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
            model->setData(index, values[row], Qt::EditRole);
    }
}

void TableInputDlg::hideSaveCancelButtons()
{
    ui->buttonBox->hide();
}

void TableInputDlg::onUndo()
{

}

void TableInputDlg::initDlg()
{
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    ui->toolButtonPasteData->setDefaultAction(ui->actionPaste);
    ui->toolButtonRedo->setDefaultAction(ui->actionRedo);
    ui->toolButtonUndo->setDefaultAction(ui->actionUndo);
}

void TableInputDlg::onPasteFromClipboard()
{
    const QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();

    qInfo()<<"testo colado"<<text;
}


void TableInputDlg::setConnections()
{
    connect(ui->toolButtonPasteData->defaultAction(), &QAction::triggered, this, &TableInputDlg::onPasteFromClipboard);
    connect(ui->toolButtonUndo->defaultAction(), &QAction::triggered, this, &TableInputDlg::onUndo);
    //connect(ui->toolButtonPasteData->defaultAction(), &QAction::triggered, this, &TableInputDlg::pasteFromClipboard);
}
