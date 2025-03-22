#pragma once

#include <QDialog>
#include <QItemDelegate>

#include <QTimer>

#include <memory.h>

#include <QAbstractItemModel>
#include <QListWidget>
#include <QItemDelegate>

#include "ProjectStructs.h"

namespace Ui {
class MapRegion;
}

class IntDelegate : public QItemDelegate {
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const  override;
};


class MapRegion : public QDialog
{
    Q_OBJECT

public:
    explicit MapRegion(QWidget *parent = nullptr,
                       int regionNumber = 1,
                       int groupNumber = 1);
    ~MapRegion();

    QList<QString> getAllZonasStr();

    void loadData(const QStringList &newAllZonasStr, std::unique_ptr<RegionData> newRegionData);

    std::unique_ptr<RegionData> getRegionData() const;

private slots:
    void addZones();
    void deleteZones();
    void onPhysicalSource();

private:
    enum rows
    {
        eMaterialZone,
        eNodes,
        eRegionSize,
        ePhysicalSource
    };

    QString vectorToString(std::vector<double> vect);

    Ui::MapRegion *ui;

    void fillTableInMaterial(QListWidgetItem *item = nullptr);

    void initDialog();

    void isCellUnique(QListWidgetItem *item);

    void setConnections();

    QStringList allZonasStr;
    std::unique_ptr<RegionData> regionData;

    QString currentMatZone;

    int regionNumber;
    int groupNumber;

    void setRegionNumber(int newRegionNumber);

};
