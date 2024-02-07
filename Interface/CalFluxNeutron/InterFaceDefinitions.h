// InterfaceConfig.h
#pragma once
#include <QPalette>
#include <QColor>

namespace Interface
{
enum eBoundaryConditionsType
{
    eVaccuo,
    ePrescribed,
    eReflective
};
enum eSaveFormat
{
    jsonFormat,
    txtFormat,
    datFormat,
};

QPalette getDarkPalette();
QPalette getLightPalette();
const char * getDefaultZoneString();
const char * getAboutApp();
const char * getZoneTableToolTip();

const int getMaxRegionQtt();

//@todo it should not be here
struct regionData
{
    //int groupNumber = 1;
    int region      = 1;
    int quote       = 50;
    int zone        = 1;
    int node        = 20;
    QString zoneStr;
    QColor materialColor = QColor(135, 206, 250);

    std::optional<std::vector<double>> sourceRight;
    std::optional<std::vector<double>> sourceLeft;
};

struct projetData
{
    eBoundaryConditionsType rightBoundaryConditionsType = eVaccuo;
    eBoundaryConditionsType leftBoundaryConditionsType  = eVaccuo; //@TBD types

    int maximumIterationsNumber = 6;
    int energyGroup = 1;
    int legendreOrder = 0;
    int quadratureOrder = 2;
    int regionNumber = 1;
    int zoneNumber = 1;

    int stopOrder = 50;

    std::string scateringFilePath;

    //They are optional because we can have reflexive bc and we don't need to set them
    std::optional<std::vector<double>> bcLeft;
    std::optional<std::vector<double>> bcRight;

    std::vector<std::shared_ptr<Interface::regionData>> regionData;

    std::array<Interface::regionData, 10> regionArray;
};
};
