#include "MapRegion.h"

#include "InterFaceDefinitions.h"
#include "TableInputDlg.h"
#include "qevent.h"
#include "qlineedit.h"
#include "ui_MapRegion.h"

#include <iostream>

#include <QMessageBox>

MapRegion::MapRegion(QWidget *parent,
                     int regionNumber,
                     int groupNumber) :
    QDialog(parent),
    ui(new Ui::MapRegion),
    regionNumber(regionNumber),
    groupNumber(groupNumber)
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

void MapRegion::addZones()
{
    if (ui->listWidget->count() < regionNumber)
    {
        auto item = new QListWidgetItem;

        item->setText(Interface::getDefaultZoneString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        ui->listWidget->addItem(item);
    }
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

void MapRegion::onTimer()
{
    qInfo()<<"ontimer";

    warningTimer.setInterval(50000);

    if (blinkTimer.isActive())
    {
        blinkTimer.stop();
    }
    else
    {
        blinkingQtt = 0;
        blinkTimer.start();
        ui->labelWarning->show();
    }
}

QString MapRegion::vectorToString(std::vector<double> vect)
{
    QString result;

    for (size_t i = 0; i < vect.size(); ++i)
    {
        result += QString::number(vect[i]);
        if (i < vect.size() - 1) {
            result += ";";
        }
    }

    return result;
}

void MapRegion::onBlink()
{
    isWarning = !isWarning;
    ++blinkingQtt;

    if (blinkingQtt >= 10)
    {
        blinkTimer.stop();
    }

    if (isWarning)
    {
        ui->labelWarning->show();
        ui->tableWidgetRegion->setStyleSheet("QTableWidget { border: 2px solid red; }");
        ui->listWidget->setStyleSheet("QListWidget { border: 2px solid red; }");
    }
    else
    {
        ui->labelWarning->hide();
        ui->listWidget->setStyleSheet("");
        ui->tableWidgetRegion->setStyleSheet("");
    }
}

void MapRegion::onPhysicalSource()
{
    TableInputDlg dlg(this);

    dlg.configTable(groupNumber, QString("Physical Source"), QString("Group"));

    if (regionData->physicalSource.has_value())
        dlg.setColumnValues(0, regionData->physicalSource.value());

    if (!dlg.exec())
        return;

    regionData->physicalSource = dlg.getColumnValues(0);

    auto pysicalSourceStr = vectorToString(regionData->physicalSource.value());
    ui->tableWidgetRegion->item(ePhysicalSource, 0)->setText(pysicalSourceStr);
}

void MapRegion::initDialog()
{
    warningTimer.start();

    blinkTimer.setInterval(700);

    ui->listWidget->setDragEnabled(true);
    ui->tableWidgetRegion->setAcceptDrops(true);

    ui->tableWidgetRegion->setToolTip(Interface::getZoneTableToolTip());

    setAcceptDrops(true);

    this->setStyleSheet("QDialog {"
                        "  border: 2px solid darkblue;"
                        "  border-radius: 10px;"
                        "}");

    ui->tableWidgetRegion->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableWidgetRegion->setItem(eMaterialZone,   0, new QTableWidgetItem(""));
    ui->tableWidgetRegion->setItem(eNodes,          0, new QTableWidgetItem(""));
    ui->tableWidgetRegion->setItem(eRegionSize,     0, new QTableWidgetItem(""));
    ui->tableWidgetRegion->setItem(ePhysicalSource, 0, new QTableWidgetItem(""));

    IntDelegate *intDelegate = new IntDelegate;
    ui->tableWidgetRegion->setItemDelegateForRow(1, intDelegate);

    ui->tableWidgetRegion->item(0, 0)->setFlags(
                ui->tableWidgetRegion->item(0, 0)->flags() & ~Qt::ItemIsEditable);
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

    connect(ui->tableWidgetRegion, &QTableWidget::cellClicked, this, [this](int row, int column)
    {
        if (row == ePhysicalSource)
            onPhysicalSource();
    });

    connect(&warningTimer, &QTimer::timeout, this,  &MapRegion::onTimer);
    connect(&blinkTimer, &QTimer::timeout, this,  &MapRegion::onBlink);

}

bool MapRegion::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Drop)
    {
        QDropEvent *dropEvent = dynamic_cast<QDropEvent*>(event);

        if (dropEvent)
        {
            //The first row is "dropble"
            QPoint dropPos = dropEvent->position().toPoint();
            int row = ui->tableWidgetRegion->rowAt(dropPos.y());

            if (row == 1
                    || row == 2)
            {
                return true;
            }
        }
    }

    return false;
}

void MapRegion::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);

    //    if (isWarning)
    //    {
    //        QPainter painter(this);

    //        QRect listRect  = ui->listWidget->geometry();
    //        QRect tableRect = ui->tableWidgetRegion->geometry();

    //        listRect.adjust(-3, -3, 3, 3);
    //        tableRect.adjust(-3, -3, 3, 3);

    //        painter.setBrush(Qt::red);
    //        painter.fillRect(listRect, Qt::red);
    //    }
}
std::unique_ptr<RegionData> MapRegion::getRegionData() const
{
    auto data = std::make_unique<RegionData>();

    auto chosenMaterial = ui->tableWidgetRegion->item(eMaterialZone, 0)->text();

    if ( (chosenMaterial != Interface::getDefaultZoneString())
         && !chosenMaterial.isEmpty() )
    {
        bool ok = false;

        auto items = ui->listWidget->findItems(chosenMaterial, Qt::MatchExactly);

        if (!items.isEmpty()) {
            auto item = items.at(0);
            data->zone = ui->listWidget->row(item) + 1;
            data->zoneStr = chosenMaterial.toStdString();
            data->physicalSource = regionData->physicalSource;
            data->quote = ui->tableWidgetRegion->item(eRegionSize, 0)->text().toDouble(&ok);
            data->node = ui->tableWidgetRegion->item(eNodes, 0)->text().toInt(&ok);
        }
        else
        {
            qWarning() << "Item was not find in the list";
        }
    }
    else
    {
        data = nullptr;
    }

    return data;
}

void MapRegion::loadRegionData(std::unique_ptr<RegionData> newRegionData)
{
    regionData = std::move(newRegionData);

    if (!regionData)
        regionData = std::make_unique<RegionData>();

    ui->tableWidgetRegion->item(eMaterialZone, 0)->setText(QString::fromStdString(regionData->zoneStr));

    ui->tableWidgetRegion->item(eNodes, 0)->setText(QString::number(regionData->node));

    ui->tableWidgetRegion->item(eRegionSize, 0)->setText(QString::number(regionData->quote));

    //Create a string with physical source
    if (regionData->physicalSource.has_value())
        ui->tableWidgetRegion->item(ePhysicalSource, 0)->setText(vectorToString(regionData->physicalSource.value()));
}

void MapRegion::loadAllZonasStr(const QStringList &newAllZonasStr)
{
    allZonasStr = newAllZonasStr;

    for (const auto& zone : allZonasStr)
    {
        auto item = new QListWidgetItem;

        item->setText(zone);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->listWidget->addItem(item);
    }
}

QList<QString> MapRegion::getAllZonasStr()
{
    allZonasStr.clear();

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        allZonasStr << ui->listWidget->item(i)->text();
    }

    return allZonasStr;
}

QWidget *IntDelegate::createEditor(QWidget *parent,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
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
    }
    else
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
