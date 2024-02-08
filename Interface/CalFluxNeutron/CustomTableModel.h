#pragma once

#include <QAbstractTableModel>

#include <QLocale>

#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QLocale>

class CustomTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CustomTableModel(QObject *parent = nullptr);

    void configTable(int row, int column,
                     QStringList &horizontalHeader,
                     QString &verticalHeaderStr);

//    void setFirstAndLastColumn(int valueFirst, int valueLast);

//    void setHeadersName(const QStringList &names);

//    void setDecimals(int value);
//    int getDecimals() const;

//    void setMinimum(double value);
//    int getMinimum() const;

//    void setMaximum(double value);
//    int getMaximum() const;

//    void setRange(double min, double max);

//    void setProjection(std::vector<double> values);
//    const std::vector<double> &getProjection() const;


    enum Column
    {
        Month,
        Grade,

        Count
    };


    // Header:
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:

    int row;
    int column;

    QStringList horizontalHeader;
    QStringList verticalHeader;
    QString projectionHeader;

    int decimals{ 2 };

    double minimum{ 0.0 };
    double maximum{ std::numeric_limits<double>::max() };

    std::vector<std::vector<double>> cellValors;

    QLocale locale;

    QVector<QMetaType::Type> columnDataTypes;

    QVariant dataDisplayRole(int row, int column) const;
    QVariant dataTextAlignmentRole(int column) const;
};

