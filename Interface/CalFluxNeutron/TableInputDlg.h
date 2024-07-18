#pragma once

#include <QDialog>
#include <QItemDelegate>
#include <QUndoStack>

#include <memory.h>
#include "CustomTableModel.h"

namespace Ui {
class TableInputDlg;
}

class DecimalDelegate : public QItemDelegate {
public:
    DecimalDelegate(QObject* parent = nullptr) : QItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override
    {
        QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
        editor->setDecimals(5);
        editor->setMinimum(0);
        editor->setMaximum(std::numeric_limits<double>::max());

        // Configura o locale do editor para usar ponto como separador decimal
        QLocale locale(QLocale::English, QLocale::UnitedStates);
        editor->setLocale(locale);

        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
        spinBox->setValue(value);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override
    {
        QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
        spinBox->interpretText();
        double value = spinBox->value();

        model->setData(index, value, Qt::EditRole);
    }
};

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

private:
    Ui::TableInputDlg *ui;

    void initDlg();

    void setConnections();

    std::unique_ptr<CustomTableModel> model;

};
