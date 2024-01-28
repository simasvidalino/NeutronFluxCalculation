#include "ActionDeleteTableData.h"

#include "DeleteTableData.h"

#include <QTableView>

const QString ActionDeleteTableData::Text = QStringLiteral("Delete");

ActionDeleteTableData::ActionDeleteTableData(QTableView *tableView, const QVariant &defaultValue)
    : QAction(Text, tableView),
      mTableView(tableView),
      mDefaultValue(defaultValue)
{
    setShortcut(QKeySequence::Delete);
    setShortcutContext(Qt::WidgetShortcut);

    connect(this, &QAction::triggered, this, &ActionDeleteTableData::deleteData);

    tableView->addAction(this);
}

void ActionDeleteTableData::deleteData()
{
    DeleteTableData deleteTableData(mTableView, mDefaultValue);

    deleteTableData.deleteData();
}
