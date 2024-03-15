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
const char * getDefaultZoneString();
const char * getAboutApp();
const char * getZoneTableToolTip();

const int getMaxRegionQtt();
};
