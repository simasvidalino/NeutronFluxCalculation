#pragma once


#include <QAction>

class QTableView;

class ActionDeleteTableData : public QAction
{
public:
    ActionDeleteTableData(QTableView *tableView, const QVariant &defaultValue = 0.0);

private:
    QTableView *mTableView;
    const QVariant mDefaultValue;


    static const QString Text;

private slots:
    void deleteData();
};


