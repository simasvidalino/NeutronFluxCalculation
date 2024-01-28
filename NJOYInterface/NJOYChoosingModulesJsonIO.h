#pragma once

#include <iostream>
#include "NJOYInterfaceDefinitions.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>

struct NJOYBase
{
    //card 1
    int pendfTape;
    int endfTape;
    int outTape1;
    int outTape2;
    
    // Card 2
    std::string matNumber = matNumberDefault;
    
};

struct NJOYBroadrInput : public NJOYBase
{
    //Card 2
    int numberOfFinalTemperature = 1;
    bool restart = false;
    bool bootstrap = false;
    double startingTemperatureFromNin = 0; //0K
    
    //Card 3
    double fractionalTolForThinning;
    long double maxEnergyforBroadeningAndThinning = 1; //1 MeV
};

struct NJOYChoosingIsotopes : public NJOYBase
{
    std::vector<std::string> fileNames;
    double precision    = precisionDefault;
};

struct NJOYModerInput: public NJOYBase
{
    ~NJOYModerInput();
    std::string input = moderTapeInputDefault;
    std::string output = moderTapeOutputDefault;

    std::optional<std::map<std::string, std::string>> inputTapesAndMat;
};

struct NJOYGrouprWeightFunctions
{
    //tbd valor opticional são aqueles que o NJOY coloca valor se eu não colocar nada e não aqueles que podem estar ou não entre as minhas escolhas.
    //Algumas escolhas excluem outras opções, essas opcões não escolhidas NÃO devem ser tsd::opcionais
    //------------   Card 8 weight function options (8a, 8b, 8c, 8d)
    //Card 8a
    int breakComputedFluxParameterAndBondarenko;
    int estimateOfPotencialScatteringCrossSection = 10;
    int maximumNumberOfComputedFluxPoint;
    int outputTapeForFluxParameter = 0;
    int indexOfRefeneceSigmaZeroInSigzArray = 0;
    int alphaForAdmixedModerator = 0;
    int admixedModeratorXSECInBarnsPerAbsorberAtom = 0;
    int heterogeneityParameter = 0;
    int alphaForExternalModerator = 0;
    int fractionOfAdmixedModeratorCrSectionInExternalModerator = 0;
    //Card 8b
    std::vector<int> weightFunction; //as tab1 record
    //Card 8c
    long double thermalBreak; //eV
    long double thermalTemperature; //eV
    long double fissionBreak;
    long double fissionTemperature;
    //Card 8d
    std::optional<double> inputResonanceFlux;
    int tapeUnitForFluxParamater;
};

struct NJOYGrouprInput : public NJOYBase
{
    int MTDOption = 0;

    //Card 2
    int neutronGroupStructure;
    int gammaGroupStructure;
    int weightFunctionOption;
    std::optional<int> weightFunctionOptionN;
    int legendreOrder;
    int numberOfTemperatures;
    int numberOfSigmaZeros;
    int longPrintOption;
    int smoothOption;

    //Card 3
    std::string title; //up to 80 characters delimited by quote

    //Card 4
    std::vector<double> temperatures = {293}; //kelvin

    //Card 5
    std::vector<double> sigmaZeroValues; //sigma zero values (including infinity)

    //------------   Card 6 --- if ign=1, read neutron group structure
    //Card 6a
    std::optional<int> numberOfNeutronGroups;
    //Crad 6b
    std::optional<std::vector<long double>> energyNeutronGroups;

    //------------   Card 7 --- if igg=1, read gamma group structure (7a and 7b)
    //Card 7a
    int numberOfGammaGroup;
    //Card 7b
    std::optional<std::vector<long double>> energyGammaGroups;

    // card 8
    NJOYGrouprWeightFunctions weightFunctions;

    //Card 9
    //    int fileToBeProcessed = 3; tbd
    //    int sectionToBeProcessed;
    std::optional<std::vector<std::string>> fileAndSectionTobeProcessed; // Ex: 3 2 'Elastic'

    //Card 10
    int nextMaterialToBeProcessed = 0;
};

struct NJOYReconrInput : public NJOYBase
{
    std::string comment = reconrCommentDefault;
    double precision    = precisionDefault;
};

struct NJOYGeneralParametersInput : public NJOYBase
{
    std::string temperatures = temperature;
    std::string reactions = reaction;
};

struct NJOYUnresInput : public NJOYBase
{
    std::string temperatures = temperature;
    std::string sigmaZero = sigma0; //Todo do better sigma zero default value

    int printOption;
};

class NJOYChoosingModulesJsonIO
{
public:
    NJOYChoosingModulesJsonIO();
    NJOYChoosingModulesJsonIO(NJOYChoosingModulesJsonIO &&);

    ~NJOYChoosingModulesJsonIO();

    void deleteBroadr();
    void deleteGroupr();
    void deleteReconr();
    void deleteUnresr();

    std::unique_ptr<NJOYModerInput>getModer();
    void setModer(std::unique_ptr<NJOYModerInput> newModer);

    std::vector<std::unique_ptr<NJOYBroadrInput> > &&getBroadr();
    void setBroadr(std::unique_ptr<NJOYBroadrInput> &&newBroadr);

    std::vector<std::unique_ptr<NJOYGrouprInput> > &&getGroupr();
    void setGroupr(std::unique_ptr<NJOYGrouprInput> &&newGroupr);

    std::vector<std::unique_ptr<NJOYReconrInput> > &&getReconr();
    void setReconr(std::unique_ptr<NJOYReconrInput> &&newReconr);

    std::vector<std::unique_ptr<NJOYUnresInput> > &&getUnresr();
    void setUnresr(std::unique_ptr<NJOYUnresInput> &&newUnresr);

private:
    std::unique_ptr<NJOYModerInput> moder;
    NJOYGeneralParametersInput general;

    std::vector<std::unique_ptr<NJOYBroadrInput>> broadr;
    std::vector<std::unique_ptr<NJOYReconrInput>> reconr;
    std::vector<std::unique_ptr<NJOYGrouprInput>> groupr;
    std::vector<std::unique_ptr<NJOYUnresInput>>  unresr;
};
