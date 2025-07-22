// InterfaceConfig.h
#pragma once
#include <QPalette>
#include <QColor>

namespace Interface
{

enum eSaveFormat
{
    jsonFormat,
    txtFormat,
    datFormat,
};

QPalette getDarkPalette();
QPalette getLightPalette();
const char * getAbsorptionChartTitle();
const char * getAbsorptionChartUnit();
const char * getAbsorptionTableUnit();
const char * getCrossSessionDataToolTip();
const char * getDefaultZoneString();
const char * getAboutApp();
const char * getIAEAAdress();
const char * getWindowTitle();
const char * getDefaultProjectName();
const char * getScalarFluxChartTitle();
const char * getScalarFluxChartUnit();
const char * getScalarFluxTableUnit();
const char * getToolTipForNodes();
const int getMaxRegionQtt();
const char * getXChart();
};
