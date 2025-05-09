#include "Worker.h"

#include <QDebug>
#include <QThread>

#include "DDNumericalMethod.h"
#include "DataMatrices.h"
#include "VariablesUsed.h"
#include "ParseFile.h"

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
        QMutexLocker lock(&locker);

        parseCrossSectionDataFileValues();

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

        emit outputData(DDResult);

        emit finished();

        DDMethod::getInstance()->destroyInstance();
        BuildMatrices::getInstance()->destroyInstance();
    }
    catch(const std::exception& e)
    {
        DDMethod::getInstance()->destroyInstance();
        BuildMatrices::getInstance()->destroyInstance();

        emit errorOccurred(e.what());
        qCritical() << "Exception inside worker thread: " << e.what();

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

    DDValues = BuildMatrices::getInstance()->copyProjectDataToRawPointers(*proj, ParseFile::getInstance()->getCrossSectionDataFileInfomation());
}

void Worker::calculateAbsorptionNeutronRatePerNode()
{
    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    BuildMatrices::getInstance()->calculateAbsorptionRatePerNode(DDValues.get(),
                                              DDResult.get());

    BuildMatrices::getInstance()->writeAbsRatePerNode(DDValues.get(), DDResult.get());
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

void Worker::parseCrossSectionDataFileValues()
{
    ParseFile::getInstance()->setProjectData(proj->energyGroup,
                                             proj->legendreOrder,
                                             proj->zoneNumber);

    if (ParseFile::ParseErrors::eOk != ParseFile::getInstance()->parseFile(proj->neutronMacroscopicCrossSectionsFilePath))
        throw std::logic_error(ParseFile::getInstance()->makeInstruction());
}

void Worker::setCancelResult()
{
    if (this->thread()->isRunning())
    {
        DDValues->iteracao.store(0, std::memory_order_relaxed);

        this->thread()->requestInterruption();
        this->thread()->quit();
        this->thread()->wait();
    }
}

