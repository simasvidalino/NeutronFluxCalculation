#pragma once

#include "NJOYChoosingModulesJsonIO.h"

#include "NJOYModulesBase.h"

struct NJOYBroadrInput;

namespace Ui {
class NJOYBroadrInputDlg;
}

class NJOYBroadrInputDlg : public NJOYModulesBase
{
    Q_OBJECT

public:
    explicit NJOYBroadrInputDlg(QWidget *parent = nullptr);
    ~NJOYBroadrInputDlg();

    std::unique_ptr<NJOYBroadrInput> save();
    void load(std::unique_ptr<NJOYBroadrInput> broadr = nullptr);

private:
    Ui::NJOYBroadrInputDlg *ui;

    void setConnections();
    void onInputTemperatures();

     QString temperatureStr;
};

