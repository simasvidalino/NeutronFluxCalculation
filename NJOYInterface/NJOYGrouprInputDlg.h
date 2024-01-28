#pragma once

#include "NJOYChoosingModulesJsonIO.h"

#include "NJOYModulesBase.h"


struct NJOYGrouprInput;

namespace Ui {
class NJOYGrouprInputDlg;
}

class NJOYGrouprInputDlg : public NJOYModulesBase
{
    Q_OBJECT

public:
    explicit NJOYGrouprInputDlg(QWidget *parent = nullptr);
    ~NJOYGrouprInputDlg();

    std::unique_ptr<NJOYGrouprInput> save();
    void load(std::unique_ptr<NJOYGrouprInput> groupr = nullptr);

public slots:
    void onOpenGrouprWeightFunctions();
    void onInputTableEGN();
    void onInputTableMFD();
    void onInputTableMTD();
    void onInputTemperatures();
    void onInputSigmaZero();

private:
    enum column
    {
        First  = 0,
        Second = 1
    };

    Ui::NJOYGrouprInputDlg *ui;

    void populateComboboxIGN();

    void populateComboboxIGG();

    void populateComboboxIsSmooth();

    void populateComboboxIPRINT();

    void populateComboboxIWT();

    void populateComboboxMFD();

    void configWidgets();

    void setConnections();

    void setToolTips();

    int numberOfTemperature;

     NJOYGrouprInput groupInput;

     int currentRowMTD = 0;
     int currentRowMFD = 0;
     int currentRIWT   = -1;

     std::optional<int> RIWTN;

     QString temperature;
     QStringList numberOfGroupBreaks;
     QStringList numberOfGammaBreaks;
};
