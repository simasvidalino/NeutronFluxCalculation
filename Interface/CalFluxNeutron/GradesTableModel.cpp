#include "GradesTableModel.h"
#include <QDebug>

GradesTableModel::GradesTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void GradesTableModel::configTable(int row, int column,
                                   QStringList &horizontalHeader,
                                   QString &verticalHeaderStr)
{
    beginResetModel();

    // Update the matrix size
    cellValors.resize(row);
    for (int i = 0; i < row; ++i) {
        cellValors[i].resize(column);
    }

    verticalHeader.clear();

    for (int iRow = 0; iRow < row; ++iRow)
        verticalHeader.push_back(verticalHeaderStr + QString::number(iRow + 1));

    this->horizontalHeader = horizontalHeader;

    this->row = row;
    this->column = column;

    endResetModel();

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));

    emit headerDataChanged(Qt::Horizontal, 0, this->column - 1);

    if (!verticalHeader.isEmpty())
        emit headerDataChanged(Qt::Vertical, 0, this->row - 1);
}

QVariant GradesTableModel::headerData(int section,
                                      Qt::Orientation orientation,
                                      int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal && section >= 0 && section < horizontalHeader.size())
            return horizontalHeader[section];

        if (orientation == Qt::Vertical && section >= 0 && section < verticalHeader.size())
            return verticalHeader[section];
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int GradesTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return row;
}

int GradesTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return column;
}

QVariant GradesTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto row = index.row();
    const auto column = index.column();

    if (role == Qt::DisplayRole)
         return locale.toString(cellValors[row][column], 'f', 2);

    if (role == Qt::EditRole)
        return cellValors[row][column];

    if (role == Qt::TextAlignmentRole)
        return dataTextAlignmentRole(column);

    return QVariant();
}

bool GradesTableModel::setData(const QModelIndex &index,
                               const QVariant &value, int role)
{
    if (data(index, role) == value)
        return false;

    bool ok = false;
    auto v = value.toDouble(&ok);

    if (!ok || v < minimum || v > maximum)
        return false;

    const auto row = index.row();
    const auto column = index.column();

    cellValors[row][column] = v;

    emit dataChanged(index, index, QVector<int>() << role);

    return true;
}

Qt::ItemFlags GradesTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant GradesTableModel::dataDisplayRole(int row, int column) const
{
    return QVariant();  // Implemente de acordo com a lógica específica do seu modelo
}

QVariant GradesTableModel::dataTextAlignmentRole(int column) const
{
    return QVariant();  // Implemente de acordo com a lógica específica do seu modelo
}
