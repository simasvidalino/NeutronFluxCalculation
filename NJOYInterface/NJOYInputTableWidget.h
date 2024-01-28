#pragma once

#include <QDialog>

namespace Ui {
class NJOYInputTableWidget;
}

class NJOYInputTableWidget : public QDialog
{
    Q_OBJECT

public:
    explicit NJOYInputTableWidget(QWidget *parent = nullptr);
    ~NJOYInputTableWidget();

    void hasItemCountity(bool hasSpinBox);

    void setExplanation(const QString explanation);

    void setTableTitle(QString title);

    void setHorizontalHeader(QString str, int column);

    QString getColumn( int column);
    QStringList getColumnList(int column);
    void setColumn(const QStringList &newData, int column, int width = 100);
    void setColumn(const QString &newData, int column, int width = 100);

    void setColumnCount(const int columnCount);
    void setRowCount(const int columnCount);
    //void setVerticalHeader(const QList <QString> headers);
    int getSelectedRow() const;
    void setCurrentRow(int nRow);

    QString getSelectedText(int column = 0);

    void setDataRange(const int min, const int max);

private:
    Ui::NJOYInputTableWidget *ui;

    QStringList data;

    void changeRowNumber(int rowCount);

    void setConnections();
};

