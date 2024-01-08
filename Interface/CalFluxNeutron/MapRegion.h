#pragma once

#include <QDialog>

#include <memory.h>

#include "VariablesUsed.h"
#include "qabstractitemmodel.h"
#include "qlistwidget.h"
#include <QItemDelegate>

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
    explicit MapRegion(QWidget *parent = nullptr);
    ~MapRegion();

    std::shared_ptr<dados_entrada> save();
    void load(std::shared_ptr<dados_entrada> choosingIso);

private slots:
    void addZones();
    void deleteZones();

private:
    Ui::MapRegion *ui;

    void initDialog();

    void isCellUnique(QListWidgetItem *item);

    void setConnections();

public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    QString currentMatZone;
};
