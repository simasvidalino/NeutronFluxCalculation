#pragma once

#include <QDialog>
#include <QItemDelegate>

#include <QTimer>

#include <memory.h>

#include <QAbstractItemModel>
#include <QListWidget>
#include <QItemDelegate>

#include "InterFaceDefinitions.h"
#include "qspinbox.h"



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
    void setAllZonasStr(const QStringList &newAllZonasStr);

    std::unique_ptr<Interface::regionData> getRegionData() const;
    void setRegionData(std::unique_ptr<Interface::regionData> newRegionData);

private slots:
    void addZones();
    void deleteZones();
    void onBlink();
    void onPhysicalSource();
    void onTimer();

private:
    enum rows
    {
        eMaterialZone,
        eNodes,
        ePhysicalSource
    };

    QString vectorToString(std::vector<double> vect);

    Ui::MapRegion *ui;

    void initDialog();

    void isCellUnique(QListWidgetItem *item);

    void setConnections();

    QStringList allZonasStr;
    std::unique_ptr<Interface::regionData> regionData;

    QString currentMatZone;

    int regionNumber;
    int groupNumber;

    QTimer warningTimer;
    QTimer blinkTimer;
    int blinkingQtt = 0;

    bool isWarning = false;

public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    void setRegionNumber(int newRegionNumber);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};
