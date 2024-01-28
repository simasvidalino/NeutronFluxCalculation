#ifndef REGIONCONFIGDLG_H
#define REGIONCONFIGDLG_H

#include <QDialog>

namespace Ui {
class RegionConfigDlg;
}

class RegionConfigDlg : public QDialog
{
    Q_OBJECT

public:
    explicit RegionConfigDlg(QWidget *parent = nullptr);
    ~RegionConfigDlg();

private:
    Ui::RegionConfigDlg *ui;
};

#endif // REGIONCONFIGDLG_H
