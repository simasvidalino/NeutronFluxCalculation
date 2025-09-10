#pragma once

#include "DataMatrices.h"
#include "InterFaceDefinitions.h"
#include "ProjectStructs.h"

class QJsonObject;
class QJsonArray;

class NeutronFlowJsonIO
{
public:
    static NeutronFlowJsonIO *getInstance();

    NeutronFlowJsonIO();

    void saveProject(Interface::eSaveFormat saveFormat = Interface::jsonFormat, QString path = "");
    void loadProject(Interface::eSaveFormat saveFormat = Interface::jsonFormat, QString path = "");

    ProjectData *getGeneralProjectDataPtr();
    std::shared_ptr<ProjectData> getGeneralProjectData();
    void setGeneralProjectData(std::shared_ptr<ProjectData> newGeneralProjectData);

    std::vector<std::shared_ptr<RegionData> > getDataPerRegion();
    void setDataPerRegion(const std::vector<std::shared_ptr<RegionData> > &newDataPerRegion);

    std::array<RegionData, 10> getRegionArray() const;

    std::shared_ptr<CalculatedData> getCalculatedData();
    void setCalculatedData(const std::shared_ptr<CalculatedData> &newCalculatedData);

protected:
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

private:
    QJsonObject saveGeneralProjectData() const;
    ProjectData loadGeneralProjectData(const QJsonObject &obj);

    QJsonObject saveCalculatedData() const;
    CalculatedData loadCalculatedData(const QJsonObject &obj);

    QJsonArray saveDataPerRegion() const;
    std::array<RegionData, 10> loadRegionArray(const QJsonArray &objArray);

    QJsonArray saveArrayPerNode(
        int group, int nodex, int totalRegionSize, const char *key, std::vector<std::vector<long double> > &vectorData) const;
    std::vector<std::vector<long double> > loadArrayPerNode(const QJsonArray &objArray);

    QJsonArray saveArrayPerGroupPerRegion(int regionNumber,
                                  int group,
                                  const char *key,
                                  std::vector<std::vector<long double> > &vectorData) const;
    std::vector<std::vector<long double> > loadArrayPerGroupPerRegion(const QJsonArray &objArray);

    QJsonArray saveArrayPerRegion(int regionNumber, const char *key, std::vector<long double> &vectorData) const;
    std::vector<long double> loadArrayPerRegion(const QJsonArray &objArray);

    static NeutronFlowJsonIO *mClass;

    std::shared_ptr<ProjectData> generalProjectData;
    std::shared_ptr<CalculatedData> calculatedData;

    const char *EnergyGroupKey                         = "EnergyGroup";
    const char *GeneralProjectDataKey                  = "GeneralProjectData";
    const char *LeftBoundaryConditionsTypeKey          = "LeftBoundaryConditionsType";
    const char *LeftBoundaryValuesKey                  = "LeftBoundaryValues";
    const char *QuadratureOrderKey                     = "QuadratureOrder";
    const char *LegendreOrderKey                       = "LegendreOrder";
    const char *MaximumIterationsNumberKey             = "MaximumIterationsNumber";
    const char *PeriodicityKey                         = "Periodicity";
    const char *PhysicalKey                            = "PhysicalFont";
    const char *ProjectFileKey                         = "ProjectFile"; //We can have all project in txt
    const char *DataPerRegionKey                       = "DataPerRegion";
    const char *RightBoundaryConditionsTypeKey         = "RightBoundaryConditionsType";
    const char *RightBoundaryValuesKey                 = "RightBoundaryValues";
    const char *ScatteringCrossSectionFileKey          = "NeutronMacroscopicCrossSections";
    const char *StopOrderKey                           = "StopOrder";
    const char *StoppingCriteriaTypeKey                = "StoppingCriteriaType";
    const char *DataVisualizationTypeKey               = "DataVisualizationType";

    const char *NodeKey                                = "NodePerRegion";
    const char *MaterialColorKey                       = "MaterialColor";
    const char *QuotaKey                               = "Quota";
    const char *RegionDataKey                          = "RegionDataKey";
    const char *ZoneNumberKey                          = "ZoneNumber";
    const char *ZoneStrKey                             = "ZoneStr";

    const char *TotalScatteringCrossSectionFilePathKey = "TotalScatteringCrossSectionFilePath";
    const char *AbsorptionCrossSectionFilePath         = "AbsorptionCrossSectionFilePath";

    const char *ScalarFluxFileKey                        = "ScalarFluxFile";

    //Results
    const char *CalculationResultsKey                = "CalculationResults";

    const char *NeutronFluxPointsPerNodeKey          = "NeutronFluxPointsPerNode";
    const char *AbsorptionRatePerNodeKey             = "AbsorptionRatePerNode";

    const char *AverageNeutronFluxPointsPerRegion          = "AverageNeutronFluxPointsPerRegion";
    const char *AverageAbsorptionRatePerRegionKey          = "AverageAbsorptionRatePerRegion";
    const char *IntegratedAbsorptionRatePerRegionKey       = "IntegratedAbsorptionRatePerRegion";
    const char *PositionKey                                = "Position";
    const char *FluxKey                                    = "ScalarNeutronFlux";
    const char *ValuesKey                                  = "Values";
    const char *ValueKey                                   = "Value";
    const char *RegionKey                                  = "RegionFlux";

    const char *TotalAbsorptionRatePerGroupPerRegionKey    = "TotalAbsorptionRatePerGroupPerRegion";
    const char *TotalScalarNeutronFluxPerGroupPerRegionKey = "TotalScalarNeutronFluxPerGroupPerRegion";
    const char *TotalAbsorptionRatePerRegionKey            = "TotalAbsorptionRatePerRegion";
    const char *TotalScalarNeutronFluxPerRegionKey         = "TotalScalarNeutronFluxPerRegion";

    //Style
    const char *StyleKey                             = "Style";
    const char *PaletteKey                           = "Palette";
    const char *ScreenModeKey                        = "ScreenMode";
    const char *FontKey                              = "Font";


};
