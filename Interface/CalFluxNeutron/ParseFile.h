#pragma once

#include <regex>
#include <string>
#include <cstdint>

#include "DataMatrices.h"

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
        eUnknowError                = 1 << 6, // 64
        eParseDisabled              = 1 << 6  // 128
    };

    friend ParseErrors operator&(ParseErrors lhs, ParseErrors rhs);
    friend ParseErrors operator|(ParseErrors lhs, ParseErrors rhs);
    friend ParseErrors& operator|=(ParseErrors& lhs, ParseErrors rhs);

    static ParseFile* getInstance();
    static std::string getErrorDescription(ParseErrors error);

    void setProjectData(int energyGroup,
                        int legendreOrder,
                        int numberOfZones);

    std::string makeInstruction();

    std::string makeExample();

    ParseErrors parseFile(std::string &fileName);

    ParseErrors parseString(std::string &str);

    CrossSectionDataFilerParameters &getCrossSectionDataFileInfomation();

    void setBSkipParse(bool newBSkipParse);
    bool getBSkipParse() const;

protected:
    int countOccurrences(std::string &str, std::string key);
    int findNumberInNextLine(std::string &input, std::regex beginPattern);
    int findLegenderOrder(const std::string &input);

private:
    ParseFile();
    ~ParseFile();

    static ParseFile* mClass;
    static const std::map<ParseErrors, std::string> errorMessages;
    CrossSectionDataFilerParameters crossSectionDataFileInfomation;

    int referenceEnergyGroup;
    int referenceLegendreOrder;
    int referenceNumberOfZones;

    ParseErrors eError = ParseErrors::eOk;

    bool bSkipParse;
};

inline ParseFile::ParseErrors operator&(ParseFile::ParseErrors lhs, ParseFile::ParseErrors rhs)
{
    return static_cast<ParseFile::ParseErrors>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

inline ParseFile::ParseErrors operator|(ParseFile::ParseErrors lhs, ParseFile::ParseErrors rhs)
{
    return static_cast<ParseFile::ParseErrors>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

inline ParseFile::ParseErrors& operator|=(ParseFile::ParseErrors& lhs, ParseFile::ParseErrors rhs)
{
    lhs = static_cast<ParseFile::ParseErrors>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    return lhs;
}
