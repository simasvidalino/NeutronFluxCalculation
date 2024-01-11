#include "ReginInformationDlg.h"
#include "ui_ReginInformationDlg.h"

ReginInformationDlg::ReginInformationDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReginInformationDlg)
{
    ui->setupUi(this);
}

ReginInformationDlg::~ReginInformationDlg()
{
    delete ui;
}
