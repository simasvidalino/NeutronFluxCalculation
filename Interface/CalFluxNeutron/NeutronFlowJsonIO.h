#pragma once

#include "InterFaceDefinitions.h"

class NeutronFlowJsonIO
{
public:
    static NeutronFlowJsonIO *getInstance();

    NeutronFlowJsonIO();

    void saveProject(Interface::eSaveFormat saveFormat = Interface::jsonFormat,
                     QString path = "");
    void loadProject(Interface::eSaveFormat saveFormat = Interface::jsonFormat,
                      QString path = "");

    std::unique_ptr<Interface::projetData> &&getGeneralProjectData();
    void setGeneralProjectData(std::unique_ptr<Interface::projetData> newGeneralProjectData);

    std::vector<std::shared_ptr<Interface::regionData> > getDataPerRegion();
    void setDataPerRegion(const std::vector<std::shared_ptr<Interface::regionData> > &newDataPerRegion);

protected:
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

private:
    std::unique_ptr<Interface::projetData> generalProjectData;
    std::vector<std::shared_ptr<Interface::regionData>> dataPerRegion;

    QJsonObject saveGeneralProjectData() const;
    Interface::projetData loadGeneralProjectData(const QJsonObject &obj);

    QJsonArray saveDataPerRegion() const;
    std::vector<std::shared_ptr<Interface::regionData>> loadDataPerRegion(const  QJsonArray &objArray);

    static NeutronFlowJsonIO* mClass;

    const char *EnergyGroupKey = "EnergyGroup";
    const char *GeneralProjectDataKey = "GeneralProjectData";
    const char *LeftBoundaryConditionsTypeKey = "LeftBoundaryConditionsType";
    const char *QuadratureOrderKey = "QuadratureOrder";
    const char *LegendreOrderKey = "LegendreOrder";
    const char *MaximumIterationsNumberKey = "MaximumIterationsNumber";
    const char *ProjectFileKey = "ProjectFile"; //We can have all project in txt
    const char *DataPerRegionKey = "DataPerRegion";
    const char *RightBoundaryConditionsTypeKey = "RightBoundaryConditionsType";
    const char *ScatteringCrossSectionFileKey = "ScatteringCrossSectionFileType";
    const char *StopOrderKey = "StopOrder";

    const char *NodeKey = "NodePerRegion";
    const char *MaterialColorKey = "MaterialColor";
    const char *QuotaKey = "Quota";
    const char *RegionDataKey = "RegionDataKey";
    const char *ZoneStrKey = "Zone";

};

