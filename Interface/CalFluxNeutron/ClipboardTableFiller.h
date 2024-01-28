#pragma once


#include <QList>
#include <QLocale>
#include <QMap>
#include <QMetaType>
#include <QModelIndex>
#include <QString>
#include <QStringList>

#include <utility>
#include <map>

class QAbstractItemModel;
class QTableView;

class ClipboardTableFiller
{
public:
  explicit ClipboardTableFiller(QTableView *tableView);
  ClipboardTableFiller(QTableView *tableView, int lastColumn);

  enum Status
  {
    Ok,
    InvalidData,
    WrongLineCount,
    WrongColumnCount,
    InvalidDataType
  };

  void setColumnType(int column, QMetaType::Type type);
  void setColumnPrecision(int column, int precision);

  Status fillTable();

private:
  QTableView *mTableView;
  QAbstractItemModel *mItemModel;
  QModelIndex mStartIndex;
  int mRowCount;
  int mColumnCount;

  QMap<int, unsigned> m_columnTypeMap;
  QMap<int, int> m_columnPrecision;

  QStringList mMessages;

  QList<QLocale> m_locales;

  QAbstractItemModel *itemModel();
  QModelIndex firstSelected();
  QModelIndex firstEditable();

  int computeRowCount();
  int computeColumnCount();
  int computeColumnCount(int lastColumn);


  void initLocales();

  Status getData(QString &data);

  void fillRow(const QString &line, int r);
  void fillCell(const QString &datum, const QModelIndex &index);

  void fillCellWithType(const QString &datum, const QModelIndex &index, unsigned type);

  void updateSelection(const QModelIndex &endIndex);

  QString fromInt(const QString &datum, bool &ok);
  QString fromDouble(int column, const QString &datum, bool &ok);
  QString fromWellFunction(const QString &datum, bool &ok);
  QString fromTubingOD(const QString &datum, bool &ok);

  void error(const QString &message);
  Status error(Status status, int row = -1, int column = -1);
};
