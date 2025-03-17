#include "MapRegion.h"

#include "CustomComboBox.h"
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

    fillTableInMaterial();
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

void MapRegion::fillTableInMaterial(QListWidgetItem *item)
{
    auto comboBox = static_cast<CustomComboBox*>(ui->tableWidgetRegion->cellWidget(eMaterialZone, 0));

    if (nullptr == comboBox)
        return;

    comboBox->blockSignals(true);

    comboBox->clear();

    // Add items from the QListWidget to the QComboBox
    for (int i = 0; i < ui->listWidget->count(); ++i)
    {
        auto *listItem = ui->listWidget->item(i);
        comboBox->addItem(listItem->text());
    }

    comboBox->blockSignals(false);

    comboBox->setCurrentText( regionData->zoneStr.c_str() );
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
    setAcceptDrops(true);

    this->setStyleSheet("QDialog {"
                        "  border: 2px solid darkblue;"
                        "  border-radius: 10px;"
                        "}");

    ui->tableWidgetRegion->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    auto *comboBox = new CustomComboBox();
    connect(comboBox, &QComboBox::currentTextChanged, this, [this](const QString &newText)
            {
        if (   (nullptr == regionData)
            || (newText.isEmpty()))
                    return;

                regionData->zoneStr = newText.toStdString();
            });

    ui->tableWidgetRegion->setCellWidget(eMaterialZone, 0, comboBox);

    ui->tableWidgetRegion->setItem(eNodes,          0, new QTableWidgetItem(""));
    ui->tableWidgetRegion->setItem(eRegionSize,     0, new QTableWidgetItem(""));
    ui->tableWidgetRegion->setItem(ePhysicalSource, 0, new QTableWidgetItem(""));

    IntDelegate *intDelegate = new IntDelegate;
    ui->tableWidgetRegion->setItemDelegateForRow(eNodes, intDelegate);
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
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Failed to add another item");
        msgBox.setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint );
        msgBox.setText("This item already exists");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.addButton(QMessageBox::Ok);
        msgBox.exec();
    }
}

void MapRegion::setConnections()
{
    connect(ui->commandLinkButtonAdd, &QCommandLinkButton::clicked, this, &MapRegion::addZones);
    connect(ui->commandLinkButtonDelete, &QCommandLinkButton::clicked, this, &MapRegion::deleteZones);

    //Save the old text and see if the new one is unique, if not, return to the old text.
    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item){
        currentMatZone = item->text();});

    connect(ui->listWidget, &QListWidget::itemChanged, this,  &MapRegion::isCellUnique);
    connect(ui->listWidget, &QListWidget::itemChanged, this,  &MapRegion::fillTableInMaterial);

    connect(ui->tableWidgetRegion, &QTableWidget::cellClicked, this, [this](int row, int column)
            {
                switch(row)
                {
                case ePhysicalSource:
                    onPhysicalSource();
                    break;
                default:
                    break;
                };
            });

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

std::unique_ptr<RegionData> MapRegion::getRegionData() const
{
    auto data = std::make_unique<RegionData>();

    auto chosenMaterial = QString::fromStdString(regionData->zoneStr);

    if ( (chosenMaterial != Interface::getDefaultZoneString())
        && !chosenMaterial.isEmpty() )
    {
        bool ok = false;

        auto items = ui->listWidget->findItems(chosenMaterial, Qt::MatchExactly);

        if (!items.isEmpty())
        {
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

void MapRegion::loadData(const QStringList &newAllZonasStr, std::unique_ptr<RegionData> newRegionData)
{
    regionData = std::move(newRegionData);
    allZonasStr = newAllZonasStr;

    if (!regionData)
        regionData = std::make_unique<RegionData>();


    for (const auto& zone : allZonasStr)
    {
        auto item = new QListWidgetItem;

        item->setText(zone);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->listWidget->addItem(item);
    }

    fillTableInMaterial();

    QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableWidgetRegion->cellWidget(eMaterialZone, 0));
    if (comboBox)
    {
        comboBox->setCurrentText(QString::fromStdString(regionData->zoneStr));
    }

    ui->tableWidgetRegion->item(eNodes, 0)->setText(QString::number(regionData->node));
    ui->tableWidgetRegion->item(eRegionSize, 0)->setText(QString::number(regionData->quote));

    //Create a string with physical source
    if (regionData->physicalSource.has_value())
        ui->tableWidgetRegion->item(ePhysicalSource, 0)->setText(vectorToString(regionData->physicalSource.value()));

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
