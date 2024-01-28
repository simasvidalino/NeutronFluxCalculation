#include "NJOYBroadrInputDlg.h"
#include "ui_NJOYBroadrInputDlg.h"

#include "NJOYChoosingModulesJsonIO.h"
#include "NJOYInputTableWidget.h"

NJOYBroadrInputDlg::NJOYBroadrInputDlg(QWidget *parent) :
    NJOYModulesBase(parent),
    ui(new Ui::NJOYBroadrInputDlg)
{
    ui->setupUi(this);

    setConnections();
}

NJOYBroadrInputDlg::~NJOYBroadrInputDlg()
{
    delete ui;
}

std::unique_ptr<NJOYBroadrInput> NJOYBroadrInputDlg::save()
{
    auto broadr = std::make_unique<NJOYBroadrInput>();

    //@Todo

    return broadr;
}

void NJOYBroadrInputDlg::load(std::unique_ptr<NJOYBroadrInput> broadr)
{
   if (nullptr == broadr)
     broadr = std::make_unique<NJOYBroadrInput>();

   //@todo
}

void NJOYBroadrInputDlg::setConnections()
{
    connect(ui->pushButtonTemperatures, &QPushButton::clicked, this,
            &NJOYBroadrInputDlg::onInputTemperatures);
}

void NJOYBroadrInputDlg::onInputTemperatures()
{
    NJOYInputTableWidget dlg(this);

    dlg.setColumnCount(1);
    dlg.setTableTitle("Temperature");
    dlg.setDataRange(1, 1000);
    dlg.setHorizontalHeader("Kelvin", 0);

    if (false == dlg.exec())
        return;

    temperatureStr = dlg.getColumn(0);
}
