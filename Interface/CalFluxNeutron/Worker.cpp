#include "Worker.h"

#include <QDebug>
#include <QMessageBox>
#include <QThread>

#include "DDNumericalMethod.h"
#include "DataMatrices.h"
#include "VariablesUsed.h"

#include <mutex>

std::mutex mtx;

Worker::Worker(QObject *parent) : QObject(parent)
{

}

Worker::~Worker()
{

}

void Worker::setProjData(ProjectData &proj)
{
    this->proj = proj;
}

void Worker::process()
{
    try
    {
        //std::unique_lock<std::mutex> lock(mtx);

        // QThread::sleep(3);

        //Update struct data
        updateDDValues();

        //Calculate scalar neutron Flux
        DDMethod::getInstance()->runDDMethodWithOneThread(*DDValues);

        copyScalarNeutronFluxToVector();

        // QThread::sleep(3);

        calculateCrossSectionMatrices();

        //Write Cross Section File
        writeCrossSectionFiles();

        //TBD
        calculateAverageFluxPerRegion();

        //Calculate Abs Rate
        calculateAbsorptionNeutronRate();

        //Write Abs Cross Section Rate and Scalar Flux Average by region
        writeCalculatedData();

        if (false == this->thread()->isInterruptionRequested())
            emit outputData(DDResult);

        //QThread::sleep(3);

        emit finished();

        DDMethod::getInstance()->destroyInstance();
    }
    catch(const std::exception& e)
    {
        DDMethod::getInstance()->destroyInstance();
        emit errorOccurred(e.what());
        qCritical() << "Invalid argument: " << e.what();

        emit finished();
    }
    catch (...)
    {
        DDMethod::getInstance()->destroyInstance();
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

    BuildMatrices matrices;

    DDValues = matrices.copyProjectDataToRawPointers(proj);
}

void Worker::calculateAbsorptionNeutronRate()
{
    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    BuildMatrices matrices;

    matrices.calculateAbsorptionRate(DDValues.get(),
                                     DDResult.get());
}

void Worker::calculateAverageFluxPerRegion()
{
    if (!DDValues)
        DDValues = std::unique_ptr<dados_entrada>();

    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    BuildMatrices matrices;

    DDResult->averageNeutronFluxPerRegion =
            matrices.calculateAverageNeutronFluxPerRegion(DDValues.get());
}

void Worker::calculateCrossSectionMatrices()
{
    if (!DDResult)
        DDResult = std::make_shared<CalculatedData>();

    BuildMatrices matrices;

    DDResult->matrices = matrices.calculateCrossSectionMatrices(DDValues.get());
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
}

void Worker::writeAverageNeutronFluxPerRegion()
{
    auto& averageNeutronFluxPerRegion = DDResult->averageNeutronFluxPerRegion;
    std::ostringstream title;
    title << "Average_Neutron_Flux_Per_Region_R" << DDValues->n_R
          << "_G"            << DDValues->G
          << "_L"            << DDValues->L
          << "_N"            << DDValues->n
          << "_Nod"          << DDValues->NODOSX
          << ".txt";
    std::string titleStr = title.str();
    std::ofstream outFile(titleStr);

    if (!outFile.is_open())
    {
        qWarning() << "Error opening file: " << titleStr;
        return;
    }

    // Iterate over zones
    for (size_t rIndex = 0; rIndex < DDValues->n_R; ++rIndex)
    {
        // Write zoneIndex as table title
        outFile << "-----------------------------------------------------------------------------------------" << std::endl;
        outFile << "Region " << rIndex + 1 << std::endl;

        // Write table headers
        outFile << std::left << std::setw(15) << "Group" << "Average Neutron Flux" << std::endl;

        for (size_t group = 0; group < DDValues->G; ++group)
        {
            outFile << std::left << std::setw(15) << group + 1
                    << DDResult->averageNeutronFluxPerRegion[rIndex][group] << std::endl;
        }

        outFile << "-----------------------------------------------------------------------------------------" << std::endl;

        // Add a newline for separation between Region if there are multiple Regions
        if (rIndex < DDResult->averageNeutronFluxPerRegion.size() - 1)
        {
            outFile << std::endl;
        }
    }

    outFile.close();
}

void Worker::writeAbsorptionRateFile()
{
    if (DDResult->absorptionRate.empty())
    {
        qWarning() << "Absorption Matrix is empty.";
        return;
    }

    auto absorptionCrossSection = DDResult->matrices.absorptionCrossSection;
    std::ostringstream title;
    title << "Absorption_Rate_R" << DDValues->n_R
          << "_G"            << DDValues->G
          << "_L"            << DDValues->L
          << "_N"            << DDValues->n
          << "_Nod"          << DDValues->NODOSX
          << ".txt";
    std::string titleStr = title.str();
    std::ofstream outFile(titleStr);

    if (!outFile.is_open())
    {
        qWarning() << "Error opening file: " << titleStr;
        return;
    }

    // Iterate over zones
    for (size_t rIndex = 0; rIndex < DDValues->n_R; ++rIndex)
    {
        // Write zoneIndex as table title
        outFile << "-----------------------------------------------------------------------------------------" << std::endl;
        outFile << "Region " << rIndex + 1 << std::endl;

        // Write table headers
        outFile << std::left << std::setw(15) << "Group" << "Absorption Rate" << std::endl;

        for (size_t group = 0; group < DDValues->G; ++group)
        {
            outFile << std::left << std::setw(15) << group + 1
                    << DDResult->absorptionRate[rIndex][group] << std::endl;
        }

        outFile << "-----------------------------------------------------------------------------------------" << std::endl;

        // Add a newline for separation between Region if there are multiple Regions
        if (rIndex < DDResult->absorptionRate.size() - 1)
        {
            outFile << std::endl;
        }
    }

    outFile.close();
}

void Worker::writeAbsorptionCrossSectionFile()
{
    if (DDResult->matrices.absorptionCrossSection.empty())
    {
        qWarning() << "Absorption Cross Section Matrix is empty.";
        return;
    }

    auto absorptionCrossSection = DDResult->matrices.absorptionCrossSection;
    auto zoneNumber  = DDValues->n_Z;
    auto groupNumber = DDValues->G;

    std::ostringstream title;
    title << "Absorption_Cross_Section_R" << DDValues->n_R
          << "_G"            << DDValues->G
          << "_L"            << DDValues->L
          << "_N"            << DDValues->n
          << "_Nod"          << DDValues->NODOSX
          << ".txt";
    std::string titleStr = title.str();
    std::ofstream outFile(titleStr);

    if (!outFile.is_open())
    {
        qWarning() << "Error opening file: " << titleStr;
        return;
    }

    // Iterate over zones
    for (size_t zoneIndex = 0; zoneIndex < zoneNumber; ++zoneIndex)
    {
        // Write zoneIndex as table title
        outFile << "Zone " << zoneIndex + 1 << std::endl;
        // Write table headers
        outFile << std::left << std::setw(15) << "Energy Group" << "Absorption Cross-Section" << std::endl;

        // Iterate over energy groups within each zone
        for (size_t group = 0; group < groupNumber; ++group)
        {
            // Write the energy group and corresponding absorption cross-section
            outFile << std::left << std::setw(15) << group + 1
                    << absorptionCrossSection[zoneIndex][group] << std::endl;
        }

        // Add a newline for separation between zones if there are multiple zones
        if (zoneIndex < DDResult->matrices.absorptionCrossSection.size() - 1)
        {
            outFile << std::endl;
        }
    }

    outFile.close();
}

void Worker::writeNeutronFluxFile()
{
    std::ofstream output;
    std::ostringstream title;
    title << "Scalar_Flux_R" << DDValues->n_R
          << "_G"            << DDValues->G
          << "_L"            << DDValues->L
          << "_N"            << DDValues->n
          << "_Nod"          << DDValues->NODOSX
          << ".txt";

    std::string titleStr = title.str();
    output.open(titleStr);

    int colWidth = 25;
    int precision = 2;

    //Write compile information
    output << std::string(colWidth * 3, '-') << std::endl;
    output << "Iteration Number: "
           <<DDValues->iteracaoFinal<<"\nTime: "<<
             DDValues->tempoFinalDeProcessamento<<"s\n";

    output << std::string(colWidth * 3, '-') << std::endl;

    // Write table headers
    output << std::left << std::setw(colWidth) << "Position x (cm)"
           << std::setw(colWidth) << "Group"
           << std::setw(colWidth) << "Scalar Neutron Flux" << std::endl;
    output << std::string(colWidth * 3, '-') << std::endl;

    double t = 0;
    int nod  = 0;
    double totalRegionSize = 0.0;

    for (int r = 0; r < DDValues->n_R; ++r)
        totalRegionSize += DDValues->TAM[r];

    while (t <= totalRegionSize)
    {
        for (int g = 0; g < DDValues->G; ++g)
        {
            if (g == 0)
            {
                output << std::left << std::setw(colWidth) << std::fixed << std::setprecision(precision) << t
                       << std::setw(colWidth) << g + 1
                       << std::fixed << std::setprecision(15) << DDValues->FLUXO_ESCALAR[g][nod] << std::endl;
            }
            else
            {
                output << std::left << std::setw(colWidth) << ""
                       << std::setw(colWidth) << g + 1
                       << std::fixed << std::setprecision(15) << DDValues->FLUXO_ESCALAR[g][nod] << std::endl;
            }
        }

        t += DDValues->periodicidade;
        nod += static_cast<int>((DDValues->NODOSX * DDValues->periodicidade) / DDValues->TAM_TOTAL);
        output << std::string(colWidth * 3, '-') << std::endl;
    }

}

void Worker::writeScatteringCrossSectionFile()
{
    if (this->thread()->isInterruptionRequested())
        return;

    if (DDResult->matrices.scatteringCrossSection.empty())
    {
        qWarning() << "Scattering Cross Section Matrix is empty.";
        return;
    }

    auto scttCrossSection = DDResult->matrices.scatteringCrossSection;
    auto zoneNumber       = DDValues->n_Z;
    auto groupNumber      = DDValues->G;

    std::ofstream output;
    std::ostringstream title;
    title << "Scattering_Cross_Section_R" << DDValues->n_R
          << "_G"            << DDValues->G
          << "_L"            << DDValues->L
          << "_N"            << DDValues->n
          << "_Nod"          << DDValues->NODOSX
          << ".txt";

    std::string titleStr = title.str();
    output.open(titleStr);

    if (!output.is_open())
    {
        qWarning() << "Error opening file: " << titleStr;
        return;
    }

    // Iterate over zones
    for (size_t zoneIndex = 0; zoneIndex < zoneNumber; ++zoneIndex)
    {
        // Write zoneIndex as table title
        output << "Zone " << zoneIndex << std::endl;
        // Write table headers
        output << std::left << std::setw(15) << "Energy Group" << "Scattering Cross-Section" << std::endl;

        // Iterate over energy groups within each zone
        for (size_t group = 0; group < groupNumber; ++group)
        {
            // Write the energy group and corresponding absorption cross-section
            output << std::left << std::setw(15) << group + 1 << scttCrossSection[zoneIndex][group] << std::endl;
        }

        // Add a newline for separation between zones if there are multiple zones
        if (zoneIndex < DDResult->matrices.absorptionCrossSection.size() - 1)
        {
            output << std::endl;
        }
    }

    output.close();
}

void Worker::setCancelResult(bool newCancelResult)
{
    this->thread()->requestInterruption();
    this->thread()->quit();
    this->thread()->wait();
}

void Worker::writeCalculatedData()
{
    if (this->thread()->isInterruptionRequested())
        return;

    writeNeutronFluxFile();
    writeAbsorptionRateFile();
    writeAverageNeutronFluxPerRegion();
}

void Worker::writeCrossSectionFiles()
{
    if (this->thread()->isInterruptionRequested())
        return;

    writeAbsorptionCrossSectionFile();
    writeScatteringCrossSectionFile();
}
