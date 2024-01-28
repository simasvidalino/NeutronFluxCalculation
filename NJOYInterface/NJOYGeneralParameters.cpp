#include "NJOYGeneralParameters.h"
#include "ui_NJOYGeneralParameters.h"

NJOYGeneralParameters::NJOYGeneralParameters(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NJOYGeneralParameters)
{
    ui->setupUi(this);
}

NJOYGeneralParameters::~NJOYGeneralParameters()
{
    delete ui;
}
