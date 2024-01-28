#include "RegionConfigDlg.h"
#include "ui_RegionConfigDlg.h"

RegionConfigDlg::RegionConfigDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegionConfigDlg)
{
    ui->setupUi(this);
}

RegionConfigDlg::~RegionConfigDlg()
{
    delete ui;
}
