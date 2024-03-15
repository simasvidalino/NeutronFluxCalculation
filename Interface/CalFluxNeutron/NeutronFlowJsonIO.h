#pragma once

#include "InterFaceDefinitions.h"
#include "ProjectStructs.h"

class NeutronFlowJsonIO
{
public:
    static NeutronFlowJsonIO *getInstance();

    NeutronFlowJsonIO();

    void saveProject(Interface::eSaveFormat saveFormat = Interface::jsonFormat,
                     QString path = "");
    void loadProject(Interface::eSaveFormat saveFormat = Interface::jsonFormat,
                      QString path = "");

    std::unique_ptr<ProjectData> &&getGeneralProjectData();
    void setGeneralProjectData(std::unique_ptr<ProjectData> newGeneralProjectData);

    std::vector<std::shared_ptr<RegionData> > getDataPerRegion();
    void setDataPerRegion(const std::vector<std::shared_ptr<RegionData> > &newDataPerRegion);

    std::array<RegionData, 10> getRegionArray() const;
    void setRegionArray(const std::array<RegionData, 10> &newRegionArray);

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

    std::unique_ptr<ProjectData> generalProjectData;
    std::array<RegionData, 10> regionArray;

    const char *EnergyGroupKey = "EnergyGroup";
    const char *GeneralProjectDataKey = "GeneralProjectData";
    const char *LeftBoundaryConditionsTypeKey = "LeftBoundaryConditionsType";
    const char *LeftBoundaryValuesKey = "LeftBoundaryValues";
    const char *QuadratureOrderKey = "QuadratureOrder";
    const char *LegendreOrderKey = "LegendreOrder";
    const char *MaximumIterationsNumberKey = "MaximumIterationsNumber";
    const char *PhysicalKey = "PhysicalFont";
    const char *ProjectFileKey = "ProjectFile"; //We can have all project in txt
    const char *DataPerRegionKey = "DataPerRegion";
    const char *RightBoundaryConditionsTypeKey = "RightBoundaryConditionsType";
    const char *RightBoundaryValuesKey = "RightBoundaryValues";
    const char *ScatteringCrossSectionFileKey = "ScatteringCrossSectionFileType";
    const char *StopOrderKey = "StopOrder";

    const char *NodeKey = "NodePerRegion";
    const char *MaterialColorKey = "MaterialColor";
    const char *QuotaKey = "Quota";
    const char *RegionDataKey = "RegionDataKey";
    const char *ZoneStrKey = "Zone";

};

