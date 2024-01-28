#include "DeleteTableData.h"

#include <QItemSelectionModel>
#include <QTableView>

DeleteTableData::DeleteTableData(QTableView *tableView, const QVariant &defaultValue)
    : mTableView(tableView),
      mDefaultValue(defaultValue)
{

}

void DeleteTableData::deleteData()
{
    auto model = mTableView->model();
    const auto selectedIndices = mTableView->selectionModel()->selectedIndexes();

    for (const auto &index : selectedIndices) {
        bool editable = index.flags() & Qt::ItemIsEditable;

        if (!editable)
            continue;


        model->setData(index, mDefaultValue);
    }
}
