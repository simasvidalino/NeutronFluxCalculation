#include "NJOYUnresrInputDlg.h"
#include "ui_NJOYUnresrInputDlg.h"

#include "NJOYChoosingModulesJsonIO.h"
#include "NJOYInputTableWidget.h"

NJOYUnresrInputDlg::NJOYUnresrInputDlg(QWidget *parent) :
    NJOYModulesBase(parent),
    ui(new Ui::NJOYUnresrInputDlg)
{
    ui->setupUi(this);

    setConnections();
}

NJOYUnresrInputDlg::~NJOYUnresrInputDlg()
{
    delete ui;
}

std::unique_ptr<NJOYUnresInput> NJOYUnresrInputDlg::save()
{
    auto unres = std::make_unique<NJOYUnresInput>();

    unres->temperatures = temperaturesStr.toStdString();
    unres->sigmaZero    = sigmaZerosStr.toStdString();
    unres->printOption  = ui->comboBoxPrintOption->currentIndex();

    return unres;
}

void NJOYUnresrInputDlg::load(std::unique_ptr<NJOYUnresInput> unresr)
{
    if (nullptr == unresr)
        unresr = std::make_unique<NJOYUnresInput>();

    temperaturesStr = QString::fromStdString(unresr->temperatures);
    sigmaZerosStr   = QString::fromStdString(unresr->sigmaZero);

    ui->comboBoxPrintOption->setCurrentIndex(unresr->printOption);
}

void NJOYUnresrInputDlg::setConnections()
{
    connect(ui->pushButtonTemperatures, &QPushButton::clicked, this,
            &NJOYUnresrInputDlg::onInputTemperatures);

    connect(ui->pushButtonSigmaZeros, &QPushButton::clicked, this,
            &NJOYUnresrInputDlg::onInputSigmaZero);
}

void NJOYUnresrInputDlg::onInputTemperatures()
{
    NJOYInputTableWidget dlg(this);

    dlg.setColumnCount(1);
    dlg.setTableTitle("Temperature");
    dlg.setDataRange(1, 1000);
    dlg.setHorizontalHeader("Kelvin", 0);
    dlg.setColumn(temperaturesStr, 0);

    if (false == dlg.exec())
        return;

    temperaturesStr = dlg.getColumn(0);
}

void NJOYUnresrInputDlg::onInputSigmaZero()
{
    NJOYInputTableWidget dlg(this);

    dlg.setColumnCount(1);
    dlg.setTableTitle("Sigma Zeros");
    dlg.setDataRange(1, 1000);
    //TBD sigma zero unit dlg.setHorizontalHeader("Kelvin", 0);

    if (false == dlg.exec())
        return;

    sigmaZerosStr = dlg.getColumn(0);
}
