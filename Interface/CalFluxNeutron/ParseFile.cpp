#include "ParseFile.h"

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>

#include <QDebug>
#include <QFile>

ParseFile * ParseFile::mClass = nullptr;

const std::map<ParseFile::ParseErrors, std::string> ParseFile::errorMessages = {
    {ParseErrors::eOk, "No errors found."},
    {ParseErrors::eNumberOfGroupDoesNotMatch, "The number of energy groups does not match."},
    {ParseErrors::eNumberOfZoneDoesNotMatch, "The number of zones does not match."},
    {ParseErrors::eNumberOfRegionDoesNotMatch, "The number of regions does not match."},
    {ParseErrors::eLegendreOrderDoesNotMatch, "The Legendre order does not match."},
    {ParseErrors::eTextIsEmpt, "The input text is empty."},
    {ParseErrors::eUnknowError, "An unknown error occurred."}
};

ParseFile *ParseFile::getInstance()
{
    if  (nullptr == mClass)
        mClass = new ParseFile();

    return mClass;
}

std::string ParseFile::getErrorDescription(ParseErrors error)
{
    auto it = errorMessages.find(error);
    
    if (it != errorMessages.end())
    {
        return it->second;
    }

    return "Unknown error.";
}

void ParseFile::setProjectData(int energyGroup,
                               int legendreOrder,
                               int numberOfZones)
{
    referenceEnergyGroup    = energyGroup;
    referencelegendreOrder  = legendreOrder;
    referenceNumberOfZones  = numberOfZones;
}

std::string ParseFile::makeInstruction()
{
    std::string instruction = "<p>Project data and material data do not match.</p>"
                              "<p><strong>To create a valid text format, follow the rules below:</strong></p> <ol>"
                              "<li><strong>Before the numerical data for material zone,</strong> start the line with <code>///</code>.</li>"
                              "<li><strong>Right after what was done in step 1,</strong> make a line identifying the total cross section starting "
                              "with <code>//</code>.</li><li><strong>Write the total cross section data.</strong></li>"
                              "<li><strong>Create the scattering matrix</strong> considering that for each degree of Legendre, you will have a "
                              "g x g matrix where g is the number of energy groups.</li></ol>";

    if ((eError & ParseFile::ParseErrors::eNumberOfGroupDoesNotMatch) == ParseFile::ParseErrors::eNumberOfGroupDoesNotMatch)
    {
        instruction.append("<p><strong>Error:</strong> " + ParseFile::getErrorDescription(ParseFile::ParseErrors::eNumberOfGroupDoesNotMatch) + "</p>");
    }
    if ((eError & ParseFile::ParseErrors::eNumberOfZoneDoesNotMatch) == ParseFile::ParseErrors::eNumberOfZoneDoesNotMatch)
    {
        instruction.append("<p><strong>Error:</strong> " + ParseFile::getErrorDescription(ParseFile::ParseErrors::eNumberOfZoneDoesNotMatch) + "</p>");
    }
    if ((eError & ParseFile::ParseErrors::eNumberOfRegionDoesNotMatch) == ParseFile::ParseErrors::eNumberOfRegionDoesNotMatch)
    {
        instruction.append("<p><strong>Error:</strong> " + ParseFile::getErrorDescription(ParseFile::ParseErrors::eNumberOfRegionDoesNotMatch) + "</p>");
    }
    if ((eError & ParseFile::ParseErrors::eLegendreOrderDoesNotMatch) == ParseFile::ParseErrors::eLegendreOrderDoesNotMatch)
    {
        instruction.append("<p><strong>Error:</strong> " + ParseFile::getErrorDescription(ParseFile::ParseErrors::eLegendreOrderDoesNotMatch) + "</p>");
    }

    return instruction;
}

ParseFile::ParseErrors ParseFile::parseFile(std::string &fileName)
{
    QFile file(fileName.c_str());

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file" << QString::fromStdString(fileName);
        return ParseErrors::eUnknowError;
    }

    QTextStream in(&file);
    std::string fileContent = in.readAll().toStdString();

    file.close();

    return parseString(fileContent);
}

ParseFile::ParseErrors ParseFile::parseString(std::string &str)
{
    eError = ParseErrors::eOk;
    std::regex beginPattern(R"(\/\/\/.*?\n\/\/)");

    auto countMaterial  = countOccurrences(str, "///");

    crossSectionDataFileInfomation.numberOfZones = countMaterial;

    if ( countMaterial != referenceNumberOfZones  )
    {
        eError |= ParseErrors::eNumberOfZoneDoesNotMatch;
    }

    crossSectionDataFileInfomation.numberOfEnergyGroup = findNumberBetween(str, beginPattern);

    if (referenceEnergyGroup > crossSectionDataFileInfomation.numberOfEnergyGroup)
    {
        eError |= ParseErrors::eNumberOfGroupDoesNotMatch;
    }

    crossSectionDataFileInfomation.numberOfLegendre = findLegenderOrder(str, beginPattern);

    if (referencelegendreOrder > crossSectionDataFileInfomation.numberOfLegendre)
    {
        eError |= ParseErrors::eLegendreOrderDoesNotMatch;
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

CrossSectionDataFilerParameters& ParseFile::getCrossSectionDataFileInfomation()
{
    return crossSectionDataFileInfomation;
}

