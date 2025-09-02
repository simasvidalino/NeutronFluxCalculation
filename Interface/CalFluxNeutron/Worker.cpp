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

        //The order metters
        calculateCrossSectionMatrices();
        calculateNeutronFluxData();
        calculateAbsorptionNeutronRateData();

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

    DDValues = BuildMatrices::getInstance()
                   ->copyProjectDataToRawPointers(*proj, ParseFile::getInstance()->getCrossSectionDataFileInfomation());
}

void Worker::calculateAbsorptionNeutronRateData()
{
    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    BuildMatrices::getInstance()->calculateAbsorptionRateData(DDValues.get(), DDResult.get());
    BuildMatrices::getInstance()->writeHTMLAbsorptionRateData(DDValues.get(), DDResult.get());
}

void Worker::calculateNeutronFluxData()
{
    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    copyScalarNeutronFluxToVector();

    BuildMatrices::getInstance()->calculateScalarNeutronFluxData(DDValues.get(), DDResult.get());
    BuildMatrices::getInstance()->writeHTMLScalarNeutronFluxData(DDValues.get(), DDResult.get());
}

void Worker::calculateCrossSectionMatrices()
{
    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    DDResult->matrices = BuildMatrices::getInstance()->calculateCrossSectionMatrices(DDValues.get());
    BuildMatrices::getInstance()->writeHTMLCrossSectionMatrices(DDValues.get(), &DDResult->matrices);
}

void Worker::copyScalarNeutronFluxToVector()
{
    if (this->thread()->isInterruptionRequested())
        return;

    // Nodes represent the vertices of the mesh, and cells represent the intervals between them.
    const int totalNode  = DDValues->NODOSX + 1;
    const int totalCells = DDValues->NODOSX;

    std::vector<std::vector<long double>> scalarFluxCellAvg(DDValues->G, std::vector<long double>(totalCells));
    std::vector<std::vector<long double>> scalarFlux(DDValues->G, std::vector<long double>(totalNode));

    //Nodal
    for (int g = 0; g < DDValues->G; ++g)
    {
        for (int nod = 0; nod < totalNode; ++nod)
        {
            scalarFlux[g][nod] = DDValues->FLUXO_ESCALAR[g][nod];
        }
    }

    //Average
    for (int g = 0; g < DDValues->G; ++g)
    {
        for (int nod = 0; nod < totalCells; ++nod)
        {
            const long double phiL = scalarFlux[g][nod];
            const long double phiR = scalarFlux[g][nod + 1];

            scalarFluxCellAvg[g][nod] =  ( 0.5L * (phiL + phiR) );
        }
    }

    DDResult->nodalScalarFlux.swap(scalarFlux);
    DDResult->cellAverageScalarFlux.swap(scalarFluxCellAvg);
}

void Worker::parseCrossSectionDataFileValues()
{
    ParseFile::getInstance()->setProjectData(proj->energyGroup,
                                             proj->legendreOrder,
                                             proj->zoneNumber);

    if (ParseFile::ParseErrors::eOk != ParseFile::getInstance()->parseFile(proj->neutronMacroscopicCrossSectionsFilePath))
    {
        throw std::logic_error(ParseFile::getInstance()->makeInstruction());
    }
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

