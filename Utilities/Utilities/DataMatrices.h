#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <cmath>
#include <sstream>
#include <vector>
#include <iomanip>
#include <memory>
#include <optional>
#include <filesystem>

#include "ProjectStructs.h"
#include "VariablesUsed.h"

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
    std::vector<std::vector<long double>> averageAbsorptionRatePerRegion;
    std::vector<std::vector<long double>> absorptionRatePerNode;
    std::vector<std::vector<long double>> integratedAbsorptionRatePerGroupPerRegion;
    std::vector<long double> integratedAbsorptionRatePerRegion;
    std::vector<long double> integratedAbsorptionRatePerZone;
    std::vector<std::vector<long double>> totalAbsorptionRatePerGroupPerRegion;

    std::vector<std::vector<long double>> nodalScalarFlux;
    std::vector<std::vector<long double>> cellAverageScalarFlux;
    std::vector<std::vector<long double>> averageNeutronFluxPerGroupPerRegion;
    std::vector<std::vector<long double>> integratedNeutronFluxPerGroupPerRegion;
    std::vector<std::vector<long double>> totalNeutronFluxPerGroupPerRegion;

    std::vector<long double> totalNeutronFluxPerRegion;
    std::vector<long double> totalAbsorptionRatePerRegion;

    std::string scalarFluxFile;
    std::string absorptionRateFile;
    std::string angularNeutronFluxFile;

    CalculatedCrossSectionMatrices matrices;

    ~CalculatedData()
    {
    }
};

class BuildMatrices
{

public:
    static BuildMatrices* getInstance();
    void destroyInstance();

    void calculateAbsorptionRateData(dados_entrada *data, CalculatedData *DDResult);
    void calculateScalarNeutronFluxData(dados_entrada *data, CalculatedData *DDResult);

    void calculateAbsorptionRatePerNode(dados_entrada *data, CalculatedData *DDResult);

    void calculateAverageAbsorptionRatePerRegion(dados_entrada *data, CalculatedData *DDResult);
    void calculateIntegratedAbsorptionRatePerRegion(dados_entrada *data, CalculatedData *DDResult);
    void calculateIntegratedAbsorptionRatePerZone(dados_entrada *data, CalculatedData *DDResult);
    void calculateTotalAbsorptionRatePerGroupPerRegion(dados_entrada *data, CalculatedData *DDResult);
    void calculateTotalAbsorptionRatePerRegion(dados_entrada *data, CalculatedData *DDResult);

    void calculateLegendreMatrix(dados_entrada* data);

    void calculateAverageNeutronFluxPerGroupPerRegion(dados_entrada *data, CalculatedData *DDResult);
    void calculateAverageNeutronFluxPerRegion(dados_entrada *data, CalculatedData *DDResult);
    void calculateIntegratedNeutronFluxPerGroupPerRegion(dados_entrada *data, CalculatedData *DDResult);
    void calculateIntegratedNeutronFluxPerZone(dados_entrada *data, CalculatedData *DDResult);
    void calculateTotalNeutronFluxPerGroupPerRegion(dados_entrada *data, CalculatedData *DDResult);
    void calculateTotalNeutronFluxPerRegion(dados_entrada *data, CalculatedData *DDResult);

    std::unique_ptr<dados_entrada> copyProjectDataToRawPointers(ProjectData &proj,
                                                                CrossSectionDataFilerParameters &fileParameter);

    CalculatedCrossSectionMatrices calculateCrossSectionMatrices(dados_entrada *data);

    std::tuple<std::vector<long double>, std::vector<long double> > getQuadratureValues(int NWanted);
    void saveQuadratureValueInCSV(const std::tuple<std::vector<long double>, std::vector<long double>> &value,
                                  int NNew,
                                  std::filesystem::path filePath);

    std::string saveMaterialData(std::string &finalPath, std::string &oldPath);

    void writeHTMLAbsorptionRateData(dados_entrada *DDValues, CalculatedData *DDResult);
    void writeHTMLScalarNeutronFluxData(dados_entrada *DDValues, CalculatedData *DDResult);

    void writeHTMLCrossSectionMatrices(dados_entrada *DDValues, CalculatedCrossSectionMatrices* matrices);

protected:
    //HTML
    virtual void writeHTMLNeutronFluxFilePerPeridiocity(dados_entrada *DDValues, CalculatedData *DDResult);
    virtual void writeHTMLAbsorptionRateFilePerPeridiocity(dados_entrada *DDValues, CalculatedData *DDResult);

    virtual void writeHTMLNeutronFluxFilePerRegionInterface(dados_entrada *DDValues, CalculatedData *DDResult);
    virtual void writeHTMLAbsorptionRateFilePerRegionInterface(dados_entrada *DDValues, CalculatedData *DDResult);

    virtual void determinePositionIncrement(std::vector<int> &nodeIndices,
                                            std::vector<double> &positions,
                                            dados_entrada *DDValues);

    virtual void writeHTMLAbsorptionCrossSectionFile(dados_entrada *DDValues, CalculatedCrossSectionMatrices *DDResult);
    virtual void writeHTMLScatteringCrossSectionFile(dados_entrada *DDValues, CalculatedCrossSectionMatrices *DDResult);

    virtual std::string computerFileName(dados_entrada *DDValues, std::string name);
    virtual std::filesystem::path computerFileNameHtm(dados_entrada *DDValues, std::string name);

private:
    BuildMatrices();

    void allocateMatrices(dados_entrada &valor);

    std::vector<std::vector<long double>> calculateAbsorptionCrossSectionMatrix(
        dados_entrada *data, std::vector<std::vector<long double>> &sigmaScattering);

    std::vector<std::vector<long double> > calculateScatteringCrossSectionMatrix(dados_entrada *data);

    void copyVectorToRawPointer(std::vector<double> &mVector, double *&buffer);
    void copyRegionVectorToRowPointers(std::array<RegionData, 10> &regionArray, int regionNumber, dados_entrada* data);
    void copyResourceToDestination(const std::string &resourcePath, const std::string &destinationPath);

    void buildCrossSectionMatrices(dados_entrada* data);

    void calculateDataMatrices(dados_entrada *data);

    std::vector<double> saveFileDataInVector();

    std::string fileName;

    int dataVisualizationType;

    dados_entrada *matricesDD_Data;
    ProjectData *projectInterfaceData;

    static BuildMatrices* m_ptr;
};
