#pragma once

#include "NJOYChoosingModulesJsonIO.h"

#include "NJOYModulesBase.h"

namespace Ui {
class NJOYGroupInputweightFunctionOptions;
}

class NJOYGroupInputweightFunctionOptions : public NJOYModulesBase
{
    Q_OBJECT

public:
    explicit NJOYGroupInputweightFunctionOptions(QWidget *parent = nullptr);
    ~NJOYGroupInputweightFunctionOptions();

    std::unique_ptr<NJOYGrouprWeightFunctions> save();
    void load(std::unique_ptr<NJOYGrouprWeightFunctions> weightFuntions);

private:
    Ui::NJOYGroupInputweightFunctionOptions *ui;

    std::vector<int> getWeightFunctionAsNumber();

    QString setWeightFunctionAsString(std::vector<int> values);

    void setConnections();

    void setTooltips();
};
