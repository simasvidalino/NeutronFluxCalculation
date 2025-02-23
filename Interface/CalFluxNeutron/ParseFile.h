#pragma once

#include <regex>
#include <string>
#include <cstdint>

class ParseFile
{
public:
    enum class ParseErrors : uint32_t
    {
        eOk                         = 0,
        eNumberOfGroupDoesNotMatch  = 1 << 1, // 2
        eNumberOfZoneDoesNotMatch   = 1 << 2, // 4
        eNumberOfRegionDoesNotMatch = 1 << 3, // 8
        eLegendreOrderDoesNotMatch  = 1 << 4, // 16
        eTextIsEmpt                 = 1 << 5, // 32
        eUnknowError                = 1 << 6  // 64
    };

    friend ParseErrors operator|(ParseErrors lhs, ParseErrors rhs);
    friend ParseErrors& operator|=(ParseErrors& lhs, ParseErrors rhs);

    static ParseFile* getInstance();

    void setProjectData(int energyGroup,
                        int legendreOrder,
                        int numberOfZones);

    ParseErrors parseString(std::string &str);

protected:
    int countOccurrences(std::string &str, std::string key);
    int findNumberBetween(std::string &input, std::regex beginPattern);
    int findLegenderOrder(std::string &input, std::regex beginPattern);

private:
    ParseFile();
    ~ParseFile();

    static ParseFile* mClass;

    int referenceEnergyGroup;
    int referencelegendreOrder;
    int referenceNumberOfZones;
    int referenceNumberOfRegion;
};

inline ParseFile::ParseErrors operator|(ParseFile::ParseErrors lhs, ParseFile::ParseErrors rhs)
{
    return static_cast<ParseFile::ParseErrors>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

inline ParseFile::ParseErrors& operator|=(ParseFile::ParseErrors& lhs, ParseFile::ParseErrors rhs)
{
    lhs = static_cast<ParseFile::ParseErrors>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    return lhs;
}
