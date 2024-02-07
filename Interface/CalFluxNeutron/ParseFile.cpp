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

    auto countMaterial  = countOccurrences(str, "///");

    if (countMaterial != referenceNumberOfZones)
        eError = ParseErrors::eNumberOfZoneDoesNotMatch;

    auto numberOfGroup = findNumberBetween(str, beginPattern);

    if (referenceEnergyGroup != numberOfGroup)
        eError = ParseErrors::eNumberOfGroupDoesNotMatch;

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
    // Find the section starting with the pattern specified by 'beginPattern'
    if (std::regex_search(input, match, beginPattern))
    {
        // Section begins after the first match of 'beginPattern'
        std::string::const_iterator searchStart = match.suffix().first;
        // Looking for the next occurrence of '//' to determine the end of the section
        std::regex endPattern(R"(\/\/)");

        if (std::regex_search(searchStart, input.cend(), match, endPattern))
        {
            // Extract the text between 'beginPattern' and the next '//'
            std::string section = std::string(searchStart, match.prefix().second);

            // Adjust the regular expression to match floating-point numbers
            std::regex numberPattern(R"(\b\d+\.\d+\b)");
            std::sregex_iterator it(section.begin(), section.end(), numberPattern);
            std::sregex_iterator it_end;

            qttFound = std::distance(it, it_end); // Count the numbers
        }
    }

    // Return the count of numbers found
    return qttFound;
}


ParseFile::ParseFile()
{

}

ParseFile::~ParseFile()
{

}
