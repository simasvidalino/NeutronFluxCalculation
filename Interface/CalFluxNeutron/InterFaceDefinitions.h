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
const char * getCrossSessionDataToolTip();
const char * getDefaultZoneString();
const char * getAboutApp();
const char * getIAEAAdress();
const char * getWindowTitle();
const char * getDefaultProjectName();

const int getMaxRegionQtt();
};
