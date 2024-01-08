#include "MapRegion.h"
#include "InterFaceDefinitions.h"
#include "qevent.h"
#include "qlineedit.h"
#include "ui_MapRegion.h"
#include <iostream>
#include <QMessageBox>

MapRegion::MapRegion(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapRegion)
{
    ui->setupUi(this);
    setConnections();
    initDialog();

    qApp->installEventFilter(this);
}

MapRegion::~MapRegion()
{
    delete ui;
}

std::shared_ptr<dados_entrada> MapRegion::save()
{
    auto data = std::make_shared<dados_entrada>();
    //
    //@todo


    return data;
}

void MapRegion::load(std::shared_ptr<dados_entrada> choosingIso)
{

}

void MapRegion::addZones()
{
    auto item = new QListWidgetItem;

    item->setText(Interface::getDefaultZoneString());
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    ui->listWidget->addItem(item);
}

void MapRegion::deleteZones()
{
    int lastRow = ui->listWidget->count() - 1;

    if (lastRow >= 0)
    {
        QListWidgetItem *itemToRemove = ui->listWidget->takeItem(lastRow);
        delete itemToRemove;
    }
    else
    {
        ui->tableWidgetRegion->removeCellWidget(0, 0);
    }
}

void MapRegion::initDialog()
{
    ui->listWidget->setDragEnabled(true);
    ui->tableWidgetRegion->setAcceptDrops(true);

    setAcceptDrops(true);

    this->setStyleSheet("QDialog {"
                        "  border: 2px solid darkblue;"
                        "  border-radius: 10px;"
                        "}");

    ui->tableWidgetRegion->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableWidgetRegion->setItem(0, 0, new QTableWidgetItem(""));
    ui->tableWidgetRegion->setItem(1, 0, new QTableWidgetItem(""));

    IntDelegate *intDelegate = new IntDelegate;
    ui->tableWidgetRegion->setItemDelegateForRow(1, intDelegate);

    ui->tableWidgetRegion->item(0, 0)->setFlags(ui->tableWidgetRegion->item(0, 0)->flags() & ~Qt::ItemIsEditable);
}

void MapRegion::isCellUnique(QListWidgetItem *item)
{
    bool itemUnique = true;

    auto itemRow = ui->listWidget->row(item);

    for (int iRow= 0; iRow < ui->listWidget->count(); ++iRow)
    {
        //Doesn't compare to itself
        if (itemRow != iRow)
        {
            auto otherItemText = ui->listWidget->item(iRow)->text();

            if ( otherItemText == item->text())
            {
                item->setText(currentMatZone);
                itemUnique = false;
                break;
            }
        }
    }

    if (!itemUnique)
        QMessageBox::warning(this, "Failed to add another item", "This item already exists");
}

void MapRegion::setConnections()
{
    connect(ui->commandLinkButtonAdd, &QCommandLinkButton::clicked, this, &MapRegion::addZones);
    connect(ui->commandLinkButtonDelete, &QCommandLinkButton::clicked, this, &MapRegion::deleteZones);

    //Save the old text and see if the new one is unique, if not, return to the old text.
    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item){
        currentMatZone = item->text();});
    connect(ui->listWidget, &QListWidget::itemChanged, this,  &MapRegion::isCellUnique);
}
bool MapRegion::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Drop)
    {
        QDropEvent *dropEvent = dynamic_cast<QDropEvent*>(event);

        if (dropEvent)
        {
            //The first row is not "dropble"
            QPoint dropPos = dropEvent->position().toPoint();
            int row = ui->tableWidgetRegion->rowAt(dropPos.y());

            if (row == 1)
            {
                return true;
            }
        }
    }

    return false;
}


QWidget *IntDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.row() == 1 && index.column() == 0)
    {
        QLineEdit *editor = new QLineEdit(parent);
        editor->setValidator(new QIntValidator(parent));
        return editor;
    }

    return IntDelegate::createEditor(parent, option, index);
}

void IntDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.row() == 1 && index.column() == 0)
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
        if (lineEdit)
        {
            lineEdit->setText(value);
        }
    } else
    {
        IntDelegate::setEditorData(editor, index);
    }
}

void IntDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.row() == 1 && index.column() == 0)
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
        if (lineEdit)
        {
            model->setData(index, lineEdit->text(), Qt::EditRole);
        }
    }
    else
    {
        IntDelegate::setModelData(editor, model, index);
    }
}
