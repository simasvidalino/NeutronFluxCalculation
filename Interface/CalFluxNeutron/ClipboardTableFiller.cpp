#include "ClipboardTableFiller.h"

#include <QAbstractItemModel>
#include <QClipboard>
#include <QGuiApplication>
#include <QLocale>
#include <QMessageBox>
#include <QMimeData>
#include <QItemSelectionModel>
#include <QTableView>

ClipboardTableFiller::ClipboardTableFiller(QTableView *tableView)
    : mTableView(tableView),
      mItemModel(itemModel()),
      mStartIndex(firstSelected()),
      mRowCount(computeRowCount()),
      mColumnCount(computeColumnCount())
{
    Q_ASSERT(mTableView);
    Q_ASSERT(mItemModel);
}

ClipboardTableFiller::ClipboardTableFiller(QTableView *tableView, int lastColumn)
    : mTableView(tableView),
      mItemModel(itemModel()),
      mStartIndex(firstSelected()),
      mRowCount(computeRowCount()),
      mColumnCount(computeColumnCount(lastColumn))
{
    Q_ASSERT(mTableView);
    Q_ASSERT(mItemModel);
}

void ClipboardTableFiller::setColumnType(int column, QMetaType::Type type)
{
    if (type == QMetaType::UnknownType) {
        m_columnTypeMap.remove(column);

        return;
    }

    m_columnTypeMap[column] = type;
}

void ClipboardTableFiller::setColumnPrecision(int column, int precision)
{
    if (precision <= 0) {
        m_columnPrecision.remove(column);

        return;
    }


    m_columnPrecision[column] = precision;
}

ClipboardTableFiller::Status ClipboardTableFiller::fillTable()
{
    QString data;

    if (getData(data) != Ok)
        return InvalidData;


    QStringList lines = data.split(QLatin1Char('\n'));
    int nLines = lines.count();

    if (nLines > mRowCount)
        return error(WrongLineCount);


    int nColumns = lines[0].split(QLatin1Char('\t')).count();

    if (nColumns > mColumnCount)
        return error(WrongColumnCount);


    initLocales();

    int r = mStartIndex.row();
    for (const QString &line : lines)
        fillRow(line, r++);


    if (!mMessages.isEmpty()) {
        error(mMessages.join('\n'));

        return InvalidDataType;
    }


    int endRow = mStartIndex.row() + nLines - 1;
    int endColumn = mStartIndex.column() + nColumns - 1;

    mTableView->setFocus();
    updateSelection(mItemModel->index(endRow, endColumn));

    return Ok;
}

QAbstractItemModel *ClipboardTableFiller::itemModel()
{
    if (!mTableView)
        return nullptr;

    return mTableView->model();
}

QModelIndex ClipboardTableFiller::firstSelected()
{
    if (!mTableView || !mItemModel)
        return QModelIndex();


    QItemSelectionModel *selectionModel = mTableView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (selectedIndexes.empty())
        return firstEditable();


    return selectedIndexes.first();
}

QModelIndex ClipboardTableFiller::firstEditable()
{
    int rowCount = mItemModel->rowCount();
    int colCount = mItemModel->columnCount();

    for (int i = 0; i < rowCount; ++i) {
        for (int j = 0; j < colCount; ++j) {
            QModelIndex index = mItemModel->index(i, j);
            Qt::ItemFlags flags = mItemModel->flags(index);

            if (flags & Qt::ItemIsEnabled && flags & Qt::ItemIsEditable)
                return index;
        }
    }

    return QModelIndex();
}

int ClipboardTableFiller::computeRowCount()
{
    if (!mItemModel || !mStartIndex.isValid())
        return 0;


    return mItemModel->rowCount() - mStartIndex.row();
}

int ClipboardTableFiller::computeColumnCount()
{
    if (!mItemModel || !mStartIndex.isValid())
        return 0;


    return mItemModel->columnCount() - mStartIndex.column();
}

int ClipboardTableFiller::computeColumnCount(int lastColumn)
{
    if (!mItemModel || !mStartIndex.isValid())
        return 0;


    if (lastColumn < 0 || lastColumn >= mItemModel->columnCount())
        return computeColumnCount();


    return lastColumn - mStartIndex.column();
}

void ClipboardTableFiller::initLocales()
{
    m_locales = {
        QLocale(),
        QLocale::system(),
        QLocale(QLocale::Portuguese, QLocale::Brazil),
        QLocale("C")
    };
}

ClipboardTableFiller::Status ClipboardTableFiller::getData(QString &data)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (!mimeData->hasText()) {
        error(InvalidData);

        return InvalidData;
    }


    data = mimeData->text().trimmed();

    return Ok;
}

void ClipboardTableFiller::fillRow(const QString &line, int r)
{
    QStringList lineData = line.split(QLatin1Char('\t'));
    int c = mStartIndex.column();

    for (const QString &datum : lineData)
        fillCell(datum, mItemModel->index(r, c++));
}

void ClipboardTableFiller::fillCell(const QString &datum,
                                    const QModelIndex &index)
{
    Qt::ItemFlags flags = mItemModel->flags(index);

    if (!(flags & Qt::ItemIsEditable))
        return;


    int column = index.column();

    if (!m_columnTypeMap.contains(column)) {
        mItemModel->setData(index, datum);

        return;
    }


    unsigned type = m_columnTypeMap[column];
    fillCellWithType(datum, index, type);
}

void ClipboardTableFiller::fillCellWithType(const QString &datum, const QModelIndex &index,
                                            unsigned type)
{
    bool ok = true;
    QString strVal = datum;
    int column = index.column();

    if (type == QMetaType::Int)
        strVal = fromInt(datum, ok);

    else if (type == QMetaType::Float || type == QMetaType::Double)
        strVal = fromDouble(column, datum, ok);


    if (ok) {
        mItemModel->setData(index, strVal);
    }

    else {
        int row = index.row();

        mMessages << QString("Unrecognized data in row %1, column %2")
                            .arg(QString::number(row + 1), QString::number(column + 1));
    }
}

void ClipboardTableFiller::updateSelection(const QModelIndex &endIndex)
{
    QItemSelection selection(mStartIndex, endIndex);

    QItemSelectionModel *selectionModel = mTableView->selectionModel();
    selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
}

QString ClipboardTableFiller::fromInt(const QString &datum, bool &ok)
{
    for (const QLocale &locale : m_locales) {
        int intVal = locale.toInt(datum, &ok);

        if (ok)
            return QString::number(intVal);
    }


    return QString();
}

QString ClipboardTableFiller::fromDouble(int column, const QString &datum, bool &ok)
{
    for (const QLocale &locale : m_locales) {
        qreal realVal = locale.toDouble(datum, &ok);

        if (ok) {
            int precision = m_columnPrecision.value(column, 6);

            return QString::number(realVal, 'f', precision);
        }
    }


    return QString();
}

void ClipboardTableFiller::error(const QString &message)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Warning");
    msgBox.setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint );
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.exec();
}

ClipboardTableFiller::Status ClipboardTableFiller::error(ClipboardTableFiller::Status status,
                                                         int row, int column)
{
    QString message;

    switch (status) {
    case InvalidData:
        message = "No suitable data found on clipboard.\n"
                  "\n"
                  "HINT: Data must be a list of TAB-separated values. Copying from"
                  " a spreadsheet application should suffice.";
        break;

    case WrongLineCount:
    case WrongColumnCount:
        message = "You can not paste here because the clipboard data and the\n"
                  "pasting area are not the same size.\n"
                  "\n"
                  "Please select another cell where to paste and try again.";
        break;

    case InvalidDataType:
        message = QString("Unrecognized data in row %1, column %2")
                .arg(row + 1)
                .arg(column + 1);
        break;

    default:
        Q_UNREACHABLE();
    }

    error(message);

    return status;
}
