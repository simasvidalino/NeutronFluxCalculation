#pragma once

#include "NJOYModulesBase.h"

struct NJOYReconrInput;

namespace Ui {
class NJOYReconrInputDlg;
}

class NJOYReconrInputDlg : public NJOYModulesBase
{
    Q_OBJECT

public:
    explicit NJOYReconrInputDlg(QWidget *parent = nullptr);
    ~NJOYReconrInputDlg();

    std::unique_ptr<NJOYReconrInput> save();
    void load(std::unique_ptr<NJOYReconrInput> reconr = nullptr);

private slots:
    virtual void help() override;

private:
    Ui::NJOYReconrInputDlg *ui;

    void setConnections();
    void configWidgets();
};

