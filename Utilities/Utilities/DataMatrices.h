#pragma once

#include <iostream>
#include <string>
#include <fstream>         //trabalhar com arquivo
#include <ctime>           //para medir o tempo de execução
#include <cmath>           //para usar a função pow
#include <sstream>         //converter std::string para double, usar ostd::stringstream para o titulo de saída
#include <vector>          //para acessar funcoes como eraser e shrink_to_fit() no vector
#include <iomanip>         //para mostrar maior numero de casas decimais na tela

#include "ProjectStructs.h"
#include "VariablesUsed.h"

#include <memory>
#include <optional>

struct CrossSectionDataFilerParameters
{
    int numberOfEnergyGroup = 0;
    int numberOfZones       = 0;
    int numberOfLegendre    = 0;
};

struct CalculatedCrossSectionMatrices
{
    std::string absorptionCrossSectionFile;
    std::string totalScatteringCrossSectionFile;

    std::vector<std::vector<long double>> absorptionCrossSection;
    std::vector<std::vector<long double>> scatteringCrossSection;
};

struct CalculatedData
{
    /* Calculated data
    *******************************************************************************************
    * cumulativeNodesX       -> cumulative nodes per region, for X location reference
    * stepSize               -> spatial node dimension per region in the X direction
    * totalLength            -> total length of the X domain
    * totalNodes             -> total number of nodes in the X domain
    * angularFluxBefore      -> Neutron flux at each node, energy group, and direction to be calculated (before)
    * angularFluxAfter       -> Neutron flux at each node, energy group, and direction to be calculated (after)
    * absorptionCrossSection -> The absorption cross-section in each zone represents the probability of a neutron being
    *                            absorbed by the material. The absorption cross-section for a given energy group can be
    *                            calculated by subtracting the scattering cross-section from the total cross-section
    *                            of that group.
    * scatteringCrossSection -> Scattering cross-section in each zone represents
    *                            a measure of the probability of a neutron being deflected from its initial trajectory due to an
    *                            interaction with an atomic nucleus, without being absorbed by it.
    * smgi                   ->
    * regionSize             -> Size of each region
    *******************************************************************************************/

    std::vector<std::vector<long double>> scalarFlux;
    std::vector<std::vector<long double>> absorptionRate;
    std::vector<std::vector<long double>> absorptionRatePerNode;
    std::vector<std::vector<long double>> averageNeutronFluxPerRegion;

    std::string scalarFluxFile;
    std::string absorptionRateFile;
    std::string absorptionRatePerNodeFile;
    std::string averageNeutronFluxPerRegionFile;

    CalculatedCrossSectionMatrices matrices;

    ~CalculatedData()
    {
        std::cout<<"Delete DDResult"<<std::flush;
    };
};

class BuildMatrices
{

public:
    enum DataOriginType
    {
        eTextFileData,
        eUserInterfaceDataAndTextFile
    };

    static BuildMatrices* getInstance();
    void destroyInstance();

    BuildMatrices(dados_entrada *newDatricesDD_Data, ProjectData *newProjectInterfaceData);

    void calculateAbsorptionRatePerRegion(dados_entrada *data, CalculatedData *DDResult);
    void calculateAbsorptionRatePerNode(dados_entrada *data, CalculatedData *DDResult);
    void calculateLegendreMatrix(dados_entrada* data);

    std::vector<std::vector<long double> > calculateAverageNeutronFluxPerRegion(dados_entrada *data);

    std::unique_ptr<dados_entrada> copyProjectDataToRawPointers(ProjectData &proj,  CrossSectionDataFilerParameters& fileParameter);

    CalculatedCrossSectionMatrices calculateCrossSectionMatrices(dados_entrada *data);

    void run(int buildType, std::string file, dados_entrada &valor);

    int getEnergyGroupQtt() const;
    void setEnergyGroupQtt(int newEnergyGroupQtt);

    int getQuadratureOrder() const;
    void setQuadratureOrder(int newQuadratureOrder);

    int getStopOrder() const;
    void setStopOrder(int newStopOrder);

    int getAnisotropyOrder() const;
    void setAnisotropyOrder(int newAnisotropyOrder);

    int getRegionsNumber() const;
    void setRegionsNumber(int newRegionsNumber);

    int getMaterialNumber() const;
    void setMaterialNumber(int newMaterialNumber);

    int getIterationNumber() const;
    void setIterationNumber(int newIterationNumber);

    std::tuple<std::vector<double>, std::vector<double>> getQuadratureValues(int NWanted);
    void saveQuadratureValueInCSV(const std::tuple<std::vector<double>, std::vector<double>> &value,
                                  int NNew,
                                  std::string filePath);

    dados_entrada *getMatricesDDData() const;
    void setMatricesDDData(dados_entrada *newMatricesDDData);

    std::string saveMaterialData(std::string &finalPath, std::string &oldPath);

    void writeAbsRatePerNode(dados_entrada *DDValues, CalculatedData *DDResult);
    void writeAverageNeutronFluxPerRegion(dados_entrada *DDValues, CalculatedData *DDResult);
    void writeAbsorptionRateFile(dados_entrada *DDValues, CalculatedData *DDResult);
    void writeNeutronFluxFile(dados_entrada *DDValues, CalculatedData *DDResult);

    void writeAbsorptionCrossSectionFile(dados_entrada *DDValues, CalculatedCrossSectionMatrices *DDResult);
    void writeScatteringCrossSectionFile(dados_entrada *DDValues, CalculatedCrossSectionMatrices *DDResult);

protected:
    virtual std::string computerFileName(dados_entrada *DDValues, std::string name);

private:
    BuildMatrices();

    void allocateMatrices(dados_entrada &valor);
    void allocateMatricesWithUserInterfaceData(dados_entrada &valor);

    std::vector<std::vector<long double> > calculateAbsorptionCrossSectionMatrix(dados_entrada *data,
                                                                                 std::vector<std::vector<long double>>& sigmaScattering);

    std::vector<std::vector<long double> > calculateScatteringCrossSectionMatrix(dados_entrada *data);

    void copyVectorToRawPointer(std::vector<double> &mVector, double *&buffer);
    void copyRegionVectorToRowPointers(std::array<RegionData, 10> &regionArray, int regionNumber, dados_entrada* data);
    void copyResourceToDestination(const std::string &resourcePath, const std::string &destinationPath);

    void buildCrossSectionMatrices(dados_entrada* data);

    void calculateDataMatrices(dados_entrada *data);

    std::vector<double> saveFileDataInVector();

    std::string fileName;
    int energyGroupQtt  = 0;
    int quadratureOrder = 0;
    int stopOrder       = 0;
    int anisotropyOrder = 0;
    int regionsNumber   = 0;
    int materialNumber  = 0;
    int iterationNumber = 0;
    int zoneNumber      = 0;
    int periodicity     = 0;
    int leftBCType      = 0;
    int rightBCType     = 0;

    dados_entrada *matricesDD_Data;
    ProjectData *projectInterfaceData;

    static BuildMatrices* m_ptr;
};
