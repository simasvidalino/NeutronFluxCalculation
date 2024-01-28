#pragma once

#include <QDialog>
#include <QUndoStack>

#include <memory.h>
#include "GradesTableModel.h"

namespace Ui {
class TableInputDlg;
}

class TableInputDlg : public QDialog
{
    Q_OBJECT

public:
    explicit TableInputDlg(QWidget *parent = nullptr);

    ~TableInputDlg();

    void configTable(int row, int column, QStringList &horizontalHeaders, QString &verticalHeader);
    void configTable(int row, QString columnHeader, QString rowHeader);    //for one column

    std::vector<double> getColumnValues(int column);
    void setColumnValues(int column, std::vector<double> values);

    void hideSaveCancelButtons();

private slots:
    void onPasteFromClipboard();
    void onUndo();

private:
    Ui::TableInputDlg *ui;

    void initDlg();

    void setConnections();

    QUndoStack *undoStack;

    std::unique_ptr<GradesTableModel> model;

};
