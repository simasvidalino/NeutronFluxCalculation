#ifndef NJOYMACROCROSSSECTIONCALCULATOR_H
#define NJOYMACROCROSSSECTIONCALCULATOR_H

#include <vector>


class NJOYMacroCrossSectionCalculator
{
public:
    NJOYMacroCrossSectionCalculator();

    void setMaterialDensity(double newMaterialDensity);

    void setAtomicMass(double newAtomicMass);

    std::vector<double> getMacroCrossSection() const;

private:
    double materialDensity = 0;
    double atomicMass = 0;
    std::vector <double> macroCrossSection;

    double N;

    void calculateMacroCrossSection();
    void calculateNumberDensity();
};

#endif // NJOYMACROCROSSSECTIONCALCULATOR_H
