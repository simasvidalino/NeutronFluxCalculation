#include "NJOYMacroCrossSectionCalculator.h"
#include <algorithm>

NJOYMacroCrossSectionCalculator::NJOYMacroCrossSectionCalculator()
{
}

void NJOYMacroCrossSectionCalculator::setMaterialDensity(double newMaterialDensity)
{
    materialDensity = newMaterialDensity;
}

void NJOYMacroCrossSectionCalculator::setAtomicMass(double newAtomicMass)
{
    atomicMass = newAtomicMass;
}

std::vector<double> NJOYMacroCrossSectionCalculator::getMacroCrossSection() const
{
    return macroCrossSection;
}

void NJOYMacroCrossSectionCalculator::calculateMacroCrossSection()
{
    calculateNumberDensity();

    std::for_each(macroCrossSection.begin(), macroCrossSection.end(), [&](double &value) {N*value;});
}

void NJOYMacroCrossSectionCalculator::calculateNumberDensity()
{
    const double avogadro = 6.0221409E+23;

    N = (materialDensity*avogadro)/atomicMass;
}
