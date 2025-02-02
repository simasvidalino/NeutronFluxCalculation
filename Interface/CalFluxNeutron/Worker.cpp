#include "Worker.h"

#include <QDebug>
#include <QMessageBox>
#include <QThread>

#include "DDNumericalMethod.h"
#include "DataMatrices.h"
#include "VariablesUsed.h"

#include <mutex>

std::mutex mtx;

Worker::Worker(QObject *parent)
    : QObject(parent)
{

}

Worker::~Worker()
{

}

void Worker::setProjData(ProjectData &proj)
{
    this->proj = &proj;
}

void Worker::process()
{
    try
    {
        std::unique_lock<std::mutex> lock(mtx);

        //Update struct data
        updateDDValues();

        //Calculate scalar neutron Flux
        DDMethod::getInstance()->runDDMethodWithOneThread(*DDValues);

        copyScalarNeutronFluxToVector();

        calculateCrossSectionMatrices();

        calculateAverageFluxPerRegion();

        //Calculate Abs Rate
        calculateAbsorptionNeutronRatePerRegion();

        calculateAbsorptionNeutronRatePerNode();

        if (false == this->thread()->isInterruptionRequested())
            emit outputData(DDResult);

        QThread::sleep(3);

        emit finished();

        DDMethod::getInstance()->destroyInstance();
        BuildMatrices::getInstance()->destroyInstance();
    }
    catch(const std::exception& e)
    {
        DDMethod::getInstance()->destroyInstance();
        BuildMatrices::getInstance()->destroyInstance();

        emit errorOccurred(e.what());
        qCritical() << "Invalid argument: " << e.what();

        emit finished();
    }
    catch (...)
    {
        DDMethod::getInstance()->destroyInstance();
        BuildMatrices::getInstance()->destroyInstance();

        qCritical() << "Unknown Error";

        emit finished();
    }
}

void Worker::updateDDValues()
{
    if (this->thread()->isInterruptionRequested())
        return;

    if (!DDValues)
        DDValues = std::unique_ptr<dados_entrada>();

    DDValues = BuildMatrices::getInstance()->copyProjectDataToRawPointers(*proj);
}

void Worker::calculateAbsorptionNeutronRatePerNode()
{
    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    BuildMatrices::getInstance()->calculateAbsorptionRatePerNode(DDValues.get(),
                                              DDResult.get());
}

void Worker::calculateAbsorptionNeutronRatePerRegion()
{
    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    BuildMatrices::getInstance()->calculateAbsorptionRatePerRegion(DDValues.get(),
                                     DDResult.get());

    BuildMatrices::getInstance()->writeAbsorptionRateFile(DDValues.get(), DDResult.get());
}

void Worker::calculateAverageFluxPerRegion()
{
    if (!DDValues)
        DDValues = std::unique_ptr<dados_entrada>();

    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    DDResult->averageNeutronFluxPerRegion =
            BuildMatrices::getInstance()->calculateAverageNeutronFluxPerRegion(DDValues.get());

    BuildMatrices::getInstance()->writeAverageNeutronFluxPerRegion(DDValues.get(), DDResult.get());
}

void Worker::calculateCrossSectionMatrices()
{
    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    DDResult->matrices = BuildMatrices::getInstance()->calculateCrossSectionMatrices(DDValues.get());

    BuildMatrices::getInstance()->writeAbsorptionCrossSectionFile(DDValues.get(), &DDResult->matrices);
    BuildMatrices::getInstance()->writeScatteringCrossSectionFile(DDValues.get(), &DDResult->matrices);
}

void Worker::copyScalarNeutronFluxToVector()
{
    if (this->thread()->isInterruptionRequested())
        return;

    int nodex = 0;
    std::vector<std::vector<long double>> scalarFlux;

    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    for (int rIndex = 0; rIndex < DDValues->n_R; ++rIndex)
        nodex += DDValues->n_nodos[rIndex];

    for (int g = 0; g < DDValues->G; ++g)
    {
        std::vector<long double> scalarFluxGroup;
        for (int nod = 0; nod < nodex; ++nod)
        {
            long double fluxValue = DDValues->FLUXO_ESCALAR[g][nod];
            scalarFluxGroup.push_back(fluxValue);
        }

        scalarFlux.push_back(scalarFluxGroup);
    }

    DDResult->scalarFlux.swap(scalarFlux);

    BuildMatrices::getInstance()->writeNeutronFluxFile(DDValues.get(), DDResult.get());
}

void Worker::setCancelResult(bool newCancelResult)
{
    this->thread()->requestInterruption();
    this->thread()->quit();
    this->thread()->wait();
}

