#pragma once

#include "InterFaceDefinitions.h"
#include "ProjectStructs.h"

class QJsonObject;
class QJsonArray;

class NeutronFlowJsonIO
{
public:
    static NeutronFlowJsonIO *getInstance();

    NeutronFlowJsonIO();

    void saveProject(Interface::eSaveFormat saveFormat = Interface::jsonFormat,
                     QString path = "");
    void loadProject(Interface::eSaveFormat saveFormat = Interface::jsonFormat,
                      QString path = "");

    ProjectData* getGeneralProjectDataPtr();
    std::shared_ptr<ProjectData> getGeneralProjectData();
    void setGeneralProjectData(std::shared_ptr<ProjectData> newGeneralProjectData);

    std::vector<std::shared_ptr<RegionData> > getDataPerRegion();
    void setDataPerRegion(const std::vector<std::shared_ptr<RegionData> > &newDataPerRegion);

    std::array<RegionData, 10> getRegionArray() const;

protected:
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

private:
    QJsonObject saveGeneralProjectData() const;
    ProjectData loadGeneralProjectData(const QJsonObject &obj);

    QJsonArray saveDataPerRegion() const;

    QJsonArray saveRegionArray() const;
    std::array<RegionData, 10> loadRegionArray(const  QJsonArray &objArray);

    static NeutronFlowJsonIO* mClass;

    std::shared_ptr<ProjectData> generalProjectData;

    const char *EnergyGroupKey = "EnergyGroup";
    const char *GeneralProjectDataKey = "GeneralProjectData";
    const char *LeftBoundaryConditionsTypeKey = "LeftBoundaryConditionsType";
    const char *LeftBoundaryValuesKey = "LeftBoundaryValues";
    const char *QuadratureOrderKey = "QuadratureOrder";
    const char *LegendreOrderKey = "LegendreOrder";
    const char *MaximumIterationsNumberKey = "MaximumIterationsNumber";
    const char *PeriodicityKey = "Periodicity";
    const char *PhysicalKey = "PhysicalFont";
    const char *ProjectFileKey = "ProjectFile"; //We can have all project in txt
    const char *DataPerRegionKey = "DataPerRegion";
    const char *RightBoundaryConditionsTypeKey = "RightBoundaryConditionsType";
    const char *RightBoundaryValuesKey = "RightBoundaryValues";
    const char *ScatteringCrossSectionFileKey = "NeutronMacroscopicCrossSections";
    const char *StopOrderKey = "StopOrder";

    const char *NodeKey = "NodePerRegion";
    const char *MaterialColorKey = "MaterialColor";
    const char *QuotaKey = "Quota";
    const char *RegionDataKey = "RegionDataKey";
    const char *ZoneNumberKey = "ZoneNumber";
    const char *ZoneStrKey = "ZoneStr";

    const char *TotalScatteringCrossSectionFilePathKey = "TotalScatteringCrossSectionFilePath";
    const char *AbsorptionCrossSectionFilePath = "AbsorptionCrossSectionFilePath";

    const char *ScalarFluxFileKey = "ScalarFluxFile";
    const char *AbsorptionRateFileKey = "AbsorptionRateFile";
    const char *AbsorptionRatePerNodeFileKey = "AbsorptionRatePerNodeFile";
    const char *AverageNeutronFluxPerRegionFileKey = "AverageNeutronFluxPerRegionFile";
};

