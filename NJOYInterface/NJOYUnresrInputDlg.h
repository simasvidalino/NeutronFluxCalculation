#pragma once

#include "NJOYModulesBase.h"

struct NJOYUnresInput;

namespace Ui {
class NJOYUnresrInputDlg;
}

class NJOYUnresrInputDlg : public NJOYModulesBase
{
    Q_OBJECT

public:
    explicit NJOYUnresrInputDlg(QWidget *parent = nullptr);
    ~NJOYUnresrInputDlg();

    std::unique_ptr<NJOYUnresInput> save();
    void load(std::unique_ptr<NJOYUnresInput> unresr = nullptr);

private slots:
    void onInputTemperatures();
    void onInputSigmaZero();

private:
    Ui::NJOYUnresrInputDlg *ui;

    void setConnections();

    QString sigmaZerosStr;
    QString temperaturesStr;
};

