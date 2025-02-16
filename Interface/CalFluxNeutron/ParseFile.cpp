#include "ParseFile.h"

#include <iostream>
#include <iterator>

ParseFile * ParseFile::mClass = nullptr;

ParseFile *ParseFile::getInstance()
{
    if  (nullptr == mClass)
        mClass = new ParseFile();

    return mClass;
}

void ParseFile::setProjectData(int energyGroup,
                               int legendreOrder,
                               int numberOfZones)
{
    referenceEnergyGroup    = energyGroup;
    referencelegendreOrder  = legendreOrder;
    referenceNumberOfZones  = numberOfZones;
}

ParseFile::ParseErrors ParseFile::parseString(std::string &str)
{
    ParseErrors eError = ParseErrors::eOk;
    std::regex beginPattern(R"(\/\/\/.*?\n\/\/)");

    auto countMaterial1  = countOccurrences(str, "///");
    auto countMaterial2  = countOccurrences(str, "Zon");

    if (    ( countMaterial1 != referenceNumberOfZones )
         || ( countMaterial2 != referenceNumberOfZones ) )
    {
        eError = ParseErrors::eNumberOfZoneDoesNotMatch;
    }

    auto numberOfGroup = findNumberBetween(str, beginPattern);

    if (referenceEnergyGroup != numberOfGroup)
    {
        eError = ParseErrors::eNumberOfGroupDoesNotMatch;
    }

    auto legenderOder = findLegenderOrder(str, beginPattern);

    if (referencelegendreOrder != legenderOder)
    {
        eError = ParseErrors::eLegendreOrderDoesNotMatch;
    }

    return eError;
}

int ParseFile::countOccurrences(std::string &str, std::string key)
{
    int count = 0;
    std::string::size_type pos = 0;

    while ((pos = str.find(key, pos)) != std::string::npos)
    {
        ++count;
        pos += key.length();
    }

    return count;
}

int ParseFile::findNumberBetween(std::string &input, std::regex beginPattern)
{
    int qttFound = 0;

    std::smatch match;
    if (std::regex_search(input, match, beginPattern))
    {
        std::string::const_iterator searchStart = match.suffix().first;
        std::regex endPattern(R"(\/\/)");

        if (std::regex_search(searchStart, input.cend(), match, endPattern))
        {
            std::string section = std::string(searchStart, match.prefix().second);

            std::regex numberPattern(R"([-+]?\b\d*\.?\d+([eE][-+]?\d+)?)");
            std::sregex_iterator it(section.begin(), section.end(), numberPattern);
            std::sregex_iterator it_end;

            qttFound = std::distance(it, it_end);
        }
    }

    return qttFound;
}

int ParseFile::findLegenderOrder(std::string &input, std::regex beginPattern)
{
    int legenderOrder = 0;
    std::smatch match;

    if (std::regex_search(input, match, beginPattern))
    {
        std::string::const_iterator searchStart = match.suffix().first;
        std::regex doubleSlashPattern(R"(//)");

        std::sregex_iterator it(searchStart, input.cend(), doubleSlashPattern);
        std::sregex_iterator end;

        legenderOrder = std::count_if(it, end, [&](const std::smatch& m) {
            std::string line = m.prefix().str() + m.str() + m.suffix().str();
            return line.find("///") == std::string::npos;
        });
    }

    return legenderOrder > 0 ? legenderOrder - 2 : 0;
}


ParseFile::ParseFile()
{

}

ParseFile::~ParseFile()
{

}
