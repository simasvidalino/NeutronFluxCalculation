#pragma once

#include <memory.h>

#include <QJsonObject>

#include "NJOYInterfaceDefinitions.h"

#include "NJOYChoosingModulesJsonIO.h"

class NJOYProjectJsonIO
{
public:
    static NJOYProjectJsonIO *getInstance();

    void deleteAllModules();

    bool saveProject(SaveFormat saveFormat);
    bool loadProject(SaveFormat saveFormat);

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    std::unique_ptr<NJOYChoosingIsotopes> &&getChoosingIsotopes();
    std::unique_ptr<NJOYGeneralParametersInput> &&getGeneralParameters();

    void setModule(std::unique_ptr<NJOYChoosingIsotopes> &&newChoosingIsotopes);
    void setModule(std::unique_ptr<NJOYGeneralParametersInput> &&newGeneralParameter);
    void setModule(std::unique_ptr<NJOYChoosingModulesJsonIO> &&newNJOYChoosingModulesJsonIO);

    NJOYChoosingModulesJsonIO &getChoosingModulesObj();
    std::unique_ptr<NJOYChoosingModulesJsonIO> &&getChoosingModules();

    void setChoosingModules(std::unique_ptr<NJOYChoosingModulesJsonIO> &&newChoosingModules);

private:
    NJOYProjectJsonIO();

    QJsonObject saveChoosingModules() const;
    std::unique_ptr<NJOYChoosingModulesJsonIO> loadChoosingModules(const QJsonObject &obj);

    QJsonObject saveChoosingIsotopes() const;
    NJOYChoosingIsotopes loadChoosingIsotopes(const QJsonObject &obj);

    QJsonObject saveGeneralParameters() const;
    NJOYGeneralParametersInput loadGeneralParametersInput(const QJsonObject &obj);

    QJsonArray saveGrouprInput() const;
    std::vector<std::unique_ptr<NJOYGrouprInput> > loadGrouprInput(const QJsonArray &array);

    QJsonObject saveModer() const;
    std::unique_ptr<NJOYModerInput> loadModer(const QJsonObject &obj);

    QJsonArray saveReconrInput() const;
    std::vector<std::unique_ptr<NJOYReconrInput>> loadReconrInput(const QJsonArray &array);

    QJsonArray saveUnresrInput() const;
    std::vector<std::unique_ptr<NJOYUnresInput> > loadUnresrInput(const QJsonArray &array);

    const char *ChoosingIsotopesKey = "Isotopes Path file";

    const char *ChoosingModulesKey  = "Modules";

    const char *EnergyGammaKey      = "Energy Gamma";

    const char *EnergyNeutronKey    = "Energy Neutron";

    const char *GeneralParameterKey = "General Parameters";

    const char *CommentKey          = "Comment";

    const char *InputFileKey        = "InputTape";

    const char *InputTapesAndMatKey = "Files and Mat";

    const char *LegendreOrderKey    = "Legendre Order";

    const char *LongPrintOptionKey  = "longPrintOption";
    
    const char *MatNumberKey        = "MatNumber";

    const char *NeutronGroupStructureKey = "NeutronGroupStructure";

    const char *MTDOptionKey        = "MTD Option";

    const char *NumberOfEnergyGammaKey   = "Number of Energy Gamma";

    const char *NumberOfEnergyNeutronKey = "Number of Energy Neutron";

    const char *NumberOfNeutronGroupsKey = "Number of Neutron Groups";

    const char *NumberOfSigmasZeroKey    = "Number of Sigmas Zero";

    const char *NumberOfTemperaturesKey  = "Number of Temperatures";

    const char *OutputFileKey       = "OutputTape";
    
    const char *PrecisionKey        = "Precision";

    const char *PrintOptionKey      = "PrintOption";

    const char *TemperaturesKey     = "Temperatures";

    const char *TitleKey            = "Title";

    const char *ReactionKey         = "Reation";

    const char *SigmasZeroKey       = "SigmaZero";

    const char *SmoothOptionKey     = "Smooth Option";

    //Modules
    const char *GROUPRModuleKey = "GROUPR";

    const char *MODERModuleKey  = "MODER";

    const char *RECONRModuleKey = "RECONR";

    const char *UNRESRModuleKey = "UNRESR";

    std::unique_ptr<NJOYGeneralParametersInput> generalParameters;
    std::unique_ptr<NJOYChoosingIsotopes>       choosingIsotopes;
    std::unique_ptr<NJOYChoosingModulesJsonIO>  choosingModules;

    static NJOYProjectJsonIO *m_class;
};

