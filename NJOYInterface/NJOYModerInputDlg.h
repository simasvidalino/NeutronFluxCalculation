#pragma once
#include "NJOYModulesBase.h"

struct NJOYModerInput;

namespace Ui {
class NJOYModerInputDlg;
}

class NJOYModerInputDlg : public NJOYModulesBase
{
    Q_OBJECT

public:
    explicit NJOYModerInputDlg(QWidget *parent = nullptr);
    ~NJOYModerInputDlg();

    std::unique_ptr<NJOYModerInput> save();
    void load(std::unique_ptr<NJOYModerInput> moder);

private slots:
    void onMergeFiles(bool checked);

private:
    Ui::NJOYModerInputDlg *ui;

    void setConnections();
};

