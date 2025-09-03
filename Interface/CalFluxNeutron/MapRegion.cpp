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

    QString chosenMaterial(regionData->zoneStr.c_str() );

    if (chosenMaterial != Interface::getDefaultZoneString())
    {
        comboBox->setCurrentText(chosenMaterial);
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
    this->setStyleSheet("QDialog {"
                        "  border: 2px solid darkblue;"
                        "  border-radius: 10px;"
                        "}");

    ui->tableWidgetRegion->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    auto *comboBox = new CustomComboBox();

    ui->tableWidgetRegion->setCellWidget(eMaterialZone, 0, comboBox);

    auto *nodesSpinBox = new QSpinBox();
    nodesSpinBox->setMinimum(1);
    nodesSpinBox->setToolTip(Interface::getToolTipForNodes());

    ui->tableWidgetRegion->setCellWidget(eNodes, 0, nodesSpinBox);

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
    QObject::connect(ui->commandLinkButtonAdd, &QCommandLinkButton::clicked, this, &MapRegion::addZones);
    QObject::connect(ui->commandLinkButtonDelete, &QCommandLinkButton::clicked, this, &MapRegion::deleteZones);

    //Save the old text and see if the new one is unique, if not, return to the old text.
    QObject::connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item){
        currentMatZone = item->text();});

    QObject::connect(ui->listWidget, &QListWidget::itemChanged, this,  &MapRegion::isCellUnique);
    QObject::connect(ui->listWidget, &QListWidget::itemChanged, this,  &MapRegion::fillTableInMaterial);

    QObject::connect(ui->tableWidgetRegion, &QTableWidget::cellClicked, this, [this](int row, int column)
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

std::unique_ptr<RegionData> MapRegion::getRegionData() const
{
    auto data = std::make_unique<RegionData>();
    QString chosenMaterial;
    QComboBox* comboBox = qobject_cast<QComboBox*>(ui->tableWidgetRegion->cellWidget(eMaterialZone, 0));

    if ( nullptr != comboBox)
    {
        chosenMaterial = comboBox->currentText();
    }

    if (    (chosenMaterial != Interface::getDefaultZoneString())
         && (!chosenMaterial.isEmpty()) )
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

            QSpinBox* spinBox = qobject_cast<QSpinBox*>(ui->tableWidgetRegion->cellWidget(eNodes, 0));

            if ( nullptr != spinBox)
            {
                data->node = spinBox->value();
            }
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

    if (nullptr == regionData)
        regionData = std::make_unique<RegionData>();

    if (true == allZonasStr.isEmpty())
    {
        auto item = new QListWidgetItem;

        item->setText(Interface::getDefaultZoneString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        ui->listWidget->addItem(item);
    }

    for (const auto& zone : allZonasStr)
    {
        if (false == zone.isEmpty())
        {
            auto item = new QListWidgetItem;

            item->setText(zone);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            ui->listWidget->addItem(item);
        }
    }

    fillTableInMaterial();

    QSpinBox* spinBox = qobject_cast<QSpinBox*>(ui->tableWidgetRegion->cellWidget(eNodes, 0));

    if ( nullptr != spinBox)
    {
        spinBox->setValue(regionData->node);
    }

    ui->tableWidgetRegion->item(eRegionSize, 0)->setText(QString::number(regionData->quote));

    //Create a string with physical source
    if (regionData->physicalSource.has_value())
    {
        ui->tableWidgetRegion->item(ePhysicalSource, 0)->setText(vectorToString(regionData->physicalSource.value()));
    }
}

QList<QString> MapRegion::getAllZonasStr()
{
    allZonasStr.clear();

    for (int i = 0; i < ui->listWidget->count(); ++i)
    {
        QString zoneTexts = ui->listWidget->item(i)->text();

        if (false == zoneTexts.isEmpty())
        {
            allZonasStr << zoneTexts;
        }
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
