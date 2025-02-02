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

    std::vector<std::vector<long double> > calculateAverageNeutronFluxPerRegion(dados_entrada *data);

    std::unique_ptr<dados_entrada> copyProjectDataToRawPointers(ProjectData &proj);

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

    dados_entrada *getMatricesDDData() const;
    void setMatricesDDData(dados_entrada *newMatricesDDData);

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

    void buildCrossSectionMatrices(dados_entrada* data);

    void calculateLegendreMatrix(dados_entrada *data);

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
