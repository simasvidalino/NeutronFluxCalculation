#include "NJOYReconrInputDlg.h"

#include <memory.h>

#include "NJOYInterfaceDefinitions.h"
#include "ui_NJOYReconrInputDlg.h"

#include "NJOYChoosingModulesJsonIO.h"

NJOYReconrInputDlg::NJOYReconrInputDlg(QWidget *parent) :
    NJOYModulesBase(parent),
    ui(new Ui::NJOYReconrInputDlg)
{
    ui->setupUi(this);

    setConnections();

    configWidgets();
}

NJOYReconrInputDlg::~NJOYReconrInputDlg()
{
    delete ui;
}

std::unique_ptr<NJOYReconrInput> NJOYReconrInputDlg::save()
{
    auto reconr = std::make_unique<NJOYReconrInput>();

    reconr->comment   = ui->lineEdit->text().toStdString();
    reconr->precision = ui->doubleSpinBox->value();

    return reconr;
}

void NJOYReconrInputDlg::load(std::unique_ptr<NJOYReconrInput> reconr)
{
    if (nullptr == reconr)
        reconr = std::make_unique<NJOYReconrInput>();

    ui->lineEdit->setText( QString::fromStdString(reconr->comment) );
    ui->doubleSpinBox->setValue( reconr->precision );
}

void NJOYReconrInputDlg::help()
{

}


void NJOYReconrInputDlg::setConnections()
{

}

void NJOYReconrInputDlg::configWidgets()
{
    ui->doubleSpinBox->setDecimals(decimals);
    ui->doubleSpinBox->setRange(0, maxPrecision);
    ui->doubleSpinBox->setSingleStep(precisionStep);
}



