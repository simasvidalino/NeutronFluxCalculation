#include "NJOYGaminrInputDlg.h"
#include "ui_NJOYGaminrInputDlg.h"

NJOYGaminrInputDlg::NJOYGaminrInputDlg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NJOYGaminrInputDlg)
{
    ui->setupUi(this);
}

NJOYGaminrInputDlg::~NJOYGaminrInputDlg()
{
    delete ui;
}
