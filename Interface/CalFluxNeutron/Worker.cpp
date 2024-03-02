#include "Worker.h"

#include <QDebug>
#include <QThread>

#include "DDNumericalMethold.h"
#include "DataMatrices.h"
#include "VariablesUsed.h"

#include <mutex>

std::mutex mtx;

Worker::Worker()
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
        std::unique_lock<std::mutex> lock(mtx);

        //Update struct data
        updateDDValues();

        //Resize DDResult Vectors
        //resizeDDMatricesResult();

        //Calculate scalar neutron Flux
        DD(*DDValues);

        QThread::sleep(3);

        //Update some Charts
        //emit outputData(DDResult);

        emit entrytData(DDValues);

       //Write Cross Section File
       // writeCrossSectionFiles();

//        //Calculate Abs Rate
//        calculateAbsorptionNeutronRate();

//        emit absorptionRate(DDResult->absorptionRate);

//        //Write Abs Cross Section Rate and Scalar Flux Average by region
//        writeCalculatedData();

        QThread::sleep(3);

        //Delete shared ptr from here
        DDValues.reset();
    }
    catch(const std::exception& e)
    {
        qCritical() << "Invalid argument: " << e.what();
    }
    catch (...)
    {
        qCritical() << "Unknown Error";
    }

    emit finished();
}

void Worker::updateDDValues()
{
    DDValues = std::make_shared<dados_entrada>();

    BuildMatrices matrices;

    DDValues = matrices.copyProjectDataToRawPointers(proj);
}

void Worker::calculateAbsorptionNeutronRate()
{
    DDResult = std::make_shared<CalculatedData>();

    BuildMatrices matrices;

    matrices.calculateAbsorptionRate(DDValues.get(), DDResult.get());
}

void Worker::resizeDDMatricesResult()
{
    DDResult = std::make_shared<CalculatedData>();

    BuildMatrices matrices;

    matrices.resizeDDOutputValues(*DDResult.get(), *DDValues.get());
}

void Worker::writeAverageNeutronFluxPerRegion()
{
    auto averageNeutronFluxPerRegion = DDResult->averageNeutronFluxPerRegion;
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

    outFile << std::left << std::setw(40) << "Region" << std::setw(30) << "Absorption Rate" << std::endl;
    outFile << "-----------------------------------------------------------------------------------------" << std::endl;

    for (int regionIndex = 0; regionIndex < DDValues->n_R; ++regionIndex)
    {
        // Assuming Map_R maps region index to zone index
        int zoneIndex = DDValues->Map_R[regionIndex];
        auto zoneName = proj.regionArray[regionIndex].zoneStr;
        // Assuming absorptionRate is indexed by region
        long double rate = DDResult->absorptionRate[regionIndex];

        outFile << std::left << std::setw(40) << "Zone " + std::to_string(zoneIndex) + " " + zoneName
                << std::setprecision(3) << std::setw(30) << rate << std::endl;
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
        outFile << "Zone " << zoneIndex << std::endl;
        // Write table headers
        outFile << std::left << std::setw(15) << "Energy Group" << "Absorption Cross-Section" << std::endl;

        // Iterate over energy groups within each zone
        for (size_t group = 0; group < groupNumber; ++group)
        {
            // Write the energy group and corresponding absorption cross-section
            outFile << std::left << std::setw(15) << group << absorptionCrossSection[zoneIndex][group] << std::endl;
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

    int colWidth = 15;
    int precision = 2;

    // Write table headers
    output << std::left << std::setw(colWidth) << "Position x (cm)"
           << std::setw(colWidth) << "Group"
           << std::setw(colWidth) << "Scalar Neutron Flux" << std::endl;
    output << std::string(colWidth * 3, '-') << std::endl;

    double t = 0;
    int nod  = 0;
    double totalRegionSize = 0.0;

    for (int r = 0; r < DDValues->n_R; ++r)
        totalRegionSize += DDResult->regionSize[r];

    while (t <= DDValues->TAM_TOTAL)
    {
        for (int g = 0; g < DDValues->G; ++g)
        {
            if (g == 0)
            {
                output << std::left << std::setw(colWidth) << std::fixed << std::setprecision(precision) << t
                       << std::setw(colWidth) << g + 1
                       << std::fixed << std::setprecision(6) << DDValues->FLUXO_ESCALAR[g][nod] << std::endl;
            }
            else
            {
                output << std::left << std::setw(colWidth) << ""
                       << std::setw(colWidth) << g + 1
                       << std::fixed << std::setprecision(6) << DDValues->FLUXO_ESCALAR[g][nod] << std::endl;
            }
        }

        t += DDValues->periodicidade;
        nod += static_cast<int>((DDValues->NODOSX * DDValues->periodicidade) / DDValues->TAM_TOTAL);
        output << std::string(colWidth * 3, '-') << std::endl;
    }

}

void Worker::writeScatteringCrossSectionFile()
{
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
            output << std::left << std::setw(15) << group << scttCrossSection[zoneIndex][group] << std::endl;
        }

        // Add a newline for separation between zones if there are multiple zones
        if (zoneIndex < DDResult->matrices.absorptionCrossSection.size() - 1)
        {
            output << std::endl;
        }
    }

    output.close();
}

void Worker::writeCalculatedData()
{
    writeNeutronFluxFile();
    writeAbsorptionRateFile();
    writeAverageNeutronFluxPerRegion();
}

void Worker::writeCrossSectionFiles()
{
    writeAbsorptionCrossSectionFile();
    writeScatteringCrossSectionFile();
}
