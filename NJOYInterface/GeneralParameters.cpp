#include "GeneralParameters.h"
#include "ui_GeneralParameters.h"

GeneralParameters::GeneralParameters(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralParameters)
{
    ui->setupUi(this);
}

GeneralParameters::~GeneralParameters()
{
    delete ui;
}
