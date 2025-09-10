#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

enum eBoundaryConditionsType
{
    eVaccuo,
    ePrescribed,
    eReflective
};

enum eStoppingCriteriaType
{
    eAbsoluteDiff,
    eRelativeDiff
};

enum eDataVisualizationType
{
    eConstantPeriodicity,
    eRegionInterface
};

struct RegionData
{
    //int groupNumber = 1;
    int region      = 1;
    double quote    = 50;
    int zone        = 1;
    int node        = 20;
    std::string zoneStr;
    int materialColor = 0x87CEFA;

    std::optional<std::vector<double>> physicalSource;
};

struct ProjectData
{
    eBoundaryConditionsType rightBoundaryConditionsType = eVaccuo;
    eBoundaryConditionsType leftBoundaryConditionsType  = eVaccuo;
    eStoppingCriteriaType stoppingCriteriaType          = eRelativeDiff;
    eDataVisualizationType dataVisualizationType        = eConstantPeriodicity;

    int maximumIterationsNumber = 1000;
    int energyGroup = 1;
    int legendreOrder = 0;
    int quadratureOrder = 2;
    int regionNumber = 1;
    int zoneNumber = 1;
    int stopOrder = 7;

    double periodicity = 10.0;

    std::string neutronMacroscopicCrossSectionsFilePath;

    //They are optional because we can have reflexive bc and we don't need to set them
    std::optional<std::vector<double>> bcLeft;
    std::optional<std::vector<double>> bcRight;

    std::array<RegionData, 10> regionArray;

    //Style
    std::string palette    = "Color scheme to Dark";
    std::string font       = "Arial";
    std::string screenMode = "Normal";
};
