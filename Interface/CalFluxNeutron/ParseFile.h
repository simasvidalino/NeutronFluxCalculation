#pragma once

#include <regex>
#include <string>

class ParseFile
{
public:

    enum ParseErrors
    {
        eOk,
        eNumberOfGroupDoesNotMatch,
        eNumberOfZoneDoesNotMatch,
        eNumberOfRegionDoesNotMatch,
        eLegendreOrderDoesNotMatch,
        eTextIsEmpt,
        eUnknowError
    };

    static ParseFile* getInstance();

    void setProjectData(int energyGroup,
                        int legendreOrder,
                        int numberOfZones);

    ParseErrors parseString(std::string &str);

protected:
    int countOccurrences(std::string &str, std::string key);
    int findNumberBetween(std::string &input, std::regex beginPattern);

private:
    ParseFile();
    ~ParseFile();

    static ParseFile* mClass;

    int referenceEnergyGroup;
    int referencelegendreOrder;
    int referenceNumberOfZones;
    int referenceNumberOfRegion;
};

