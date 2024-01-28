#pragma once

#include "GradesTableModel.h"
#include "VariablesUsed.h"
#include "qtableview.h"
#include <QDialog>

namespace Ui {
class CrossSectionFileDlg;
}

class CrossSectionFileDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CrossSectionFileDlg(QWidget *parent   = nullptr,
                                 QStringList materials = {},
                                 int energyGroup   = 0,
                                 int legendreOrder = 0);
    ~CrossSectionFileDlg();
    void saveCrossSectionFileDlgCrossSection(long double ****s_s);

private slots:
    void clearText();
    void openFile();
    void saveText();

private:
    Ui::CrossSectionFileDlg *ui;

    void initDlg();
    void setConnection();

     const int energyGroup;
     const int legendreOrder;

     std::unique_ptr<GradesTableModel> model;
     std::unique_ptr<QTableView> tableView;
     QStringList materialList;
};

