#pragma once

#include "DataMatrices.h"
#include "ProjectStructs.h"
#include "VariablesUsed.h"

#include <QObject>
#include <QMutex>

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker(QObject *parent = nullptr);
    virtual ~Worker();

    void setProjData(ProjectData &proj);

    void setCancelResult();

public slots:
    void process();

signals:
    void errorOccurred(const QString &error);
    void finished();
    void outputData(std::shared_ptr<CalculatedData>);
    void entrytData(std::shared_ptr<dados_entrada>); //TBD delete it
    void nodalScalarFlux(std::vector<std::vector<long double>>&);
    void averageAbsorptionRatePerRegion(std::vector<long double>);

protected:
    void updateDDValues(); //DD method depends on some row matrices.
    virtual void calculateAbsorptionNeutronRateData();
    virtual void calculateNeutronFluxData();
    virtual void calculateCrossSectionMatrices();
    void copyScalarNeutronFluxToVector();

    void parseCrossSectionDataFileValues();

private:
    ProjectData* proj;
    std::unique_ptr<dados_entrada> DDValues;
    std::shared_ptr<CalculatedData> DDResult;

    QMutex locker;

    void writeCalculatedData();
};
