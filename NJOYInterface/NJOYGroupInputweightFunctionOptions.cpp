#include "NJOYGroupInputweightFunctionOptions.h"
#include "ui_NJOYGroupInputweightFunctionOptions.h"

#include "NJOYChoosingModulesJsonIO.h"

NJOYGroupInputweightFunctionOptions::NJOYGroupInputweightFunctionOptions(QWidget *parent) :
    NJOYModulesBase(parent),
    ui(new Ui::NJOYGroupInputweightFunctionOptions)
{
    ui->setupUi(this);

    setTooltips();

    setConnections();
}

NJOYGroupInputweightFunctionOptions::~NJOYGroupInputweightFunctionOptions()
{
    delete ui;
}

std::unique_ptr<NJOYGrouprWeightFunctions> NJOYGroupInputweightFunctionOptions::save()
{
    auto weightFuntions = std::make_unique<NJOYGrouprWeightFunctions>();

    weightFuntions->admixedModeratorXSECInBarnsPerAbsorberAtom = ui->spinBoxSam->value();
    weightFuntions->alphaForAdmixedModerator = ui->spinBoxAlphaForAdmixedModerator->value();
    weightFuntions->alphaForExternalModerator = ui->spinBoxAlphaForExternalModerator->value();
    weightFuntions->fractionOfAdmixedModeratorCrSectionInExternalModerator = ui->spinBoxFractionOfAdmixed->value();
    weightFuntions->weightFunction = getWeightFunctionAsNumber();
    weightFuntions->estimateOfPotencialScatteringCrossSection = ui->spinBoxEstimateOfPotential->value();
    weightFuntions->fissionBreak = ui->spinBoxFissionBreak->value();
    weightFuntions->fissionTemperature = ui->spinBoxFissionTemperature->value();
    weightFuntions->breakComputedFluxParameterAndBondarenko = ui->spinBoxFehi->value();
    weightFuntions->maximumNumberOfComputedFluxPoint = ui->spinBoxNflmax->value();
    weightFuntions->outputTapeForFluxParameter = ui->spinBoxNinwt->value();
    weightFuntions->indexOfRefeneceSigmaZeroInSigzArray = ui->spinBoxIndexSigmaZero->value();
    //TBD weightFuntions->tapeUnitForFluxParamater = ui->spinBoxNinwt->value();
    weightFuntions->thermalBreak = ui->spinBoxThermalBreak->value();
    weightFuntions->thermalTemperature = ui->spinBoxThermalTemperature->value();

    return weightFuntions;
}

void NJOYGroupInputweightFunctionOptions::load(std::unique_ptr<NJOYGrouprWeightFunctions> weightFuntions)
{
    if (nullptr == weightFuntions)
        weightFuntions = std::make_unique<NJOYGrouprWeightFunctions>();

    ui->spinBoxSam->setValue(weightFuntions->admixedModeratorXSECInBarnsPerAbsorberAtom);
    ui->spinBoxAlphaForAdmixedModerator->setValue(weightFuntions->alphaForAdmixedModerator);
    ui->spinBoxAlphaForExternalModerator->setValue(weightFuntions->alphaForExternalModerator);
    ui->spinBoxFractionOfAdmixed->setValue(weightFuntions->fractionOfAdmixedModeratorCrSectionInExternalModerator);
    setWeightFunctionAsString(weightFuntions->weightFunction);
    ui->spinBoxEstimateOfPotential->setValue(weightFuntions->estimateOfPotencialScatteringCrossSection);
    ui->spinBoxFissionBreak->setValue(weightFuntions->fissionBreak);
    ui->spinBoxFissionTemperature->setValue(weightFuntions->fissionTemperature);
    ui->spinBoxFehi->setValue(weightFuntions->breakComputedFluxParameterAndBondarenko);
    ui->spinBoxNflmax->setValue(weightFuntions->maximumNumberOfComputedFluxPoint);
    ui->spinBoxNinwt->setValue(weightFuntions->outputTapeForFluxParameter);
    ui->spinBoxIndexSigmaZero->setValue(weightFuntions->indexOfRefeneceSigmaZeroInSigzArray);
    ui->spinBoxThermalBreak->setValue(weightFuntions->thermalBreak);
    ui->spinBoxThermalTemperature->setValue(weightFuntions->thermalTemperature);
}

std::vector<int> NJOYGroupInputweightFunctionOptions::getWeightFunctionAsNumber()
{
    std::vector<int> dataVector;
    auto data = ui->plainTextEditWeightFunction->toPlainText().split(";").toVector();

    for (const auto &item : data)
        dataVector.push_back(item.toInt());

    return dataVector;
}

QString NJOYGroupInputweightFunctionOptions::setWeightFunctionAsString(std::vector<int> values)
{
    //@todo

    QString str;

    return str;
}

void NJOYGroupInputweightFunctionOptions::setConnections()
{
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void NJOYGroupInputweightFunctionOptions::setTooltips()
{
    ui->spinBoxFehi->setToolTip(fehiToolTip);

}
