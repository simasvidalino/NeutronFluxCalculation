#pragma once

#include "ProjectStructs.h"
#include "VariablesUsed.h"

#include <QObject>

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker(QObject *parent = nullptr);
    virtual ~Worker();

    void setProjData(ProjectData &proj);

public slots:
    void process();

signals:
    void errorOccurred(const QString &error);
    void finished();
    void outputData(std::shared_ptr<CalculatedData>);
    void entrytData(std::shared_ptr<dados_entrada>); //TBD delete it
    void scalarFlux(std::vector<std::vector<long double>>&);
    void absorptionRate(std::vector<long double>);

protected:
    void updateDDValues(); //DD method depends on some row matrices.
    virtual void calculateAbsorptionNeutronRate();
    virtual void calculateAverageFluxPerRegion();
    virtual void calculateCrossSectionMatrices();
    virtual void calculateScalarNeutronFlux();
    void    copyScalarNeutronFluxToVector();
    virtual void writeAverageNeutronFluxPerRegion();
    virtual void writeAbsorptionRateFile();
    virtual void writeAbsorptionCrossSectionFile();
    virtual void writeNeutronFluxFile();
    virtual void writeScatteringCrossSectionFile();

private:
    ProjectData proj;
    std::unique_ptr<dados_entrada> DDValues;
    std::shared_ptr<CalculatedData> DDResult;
    std::vector<std::function<void()>> tasks;

    void addTasksinVector();
    void writeCalculatedData();
    void writeCrossSectionFiles();
};
