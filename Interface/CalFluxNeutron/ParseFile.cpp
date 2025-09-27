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
    std::string instruction = "<p><strong>To create a valid text format, follow the rules below:</strong></p> <ol>"
                              "<li><strong>Before the numerical data for material zone,</strong> start the line with <code>///</code>.</li>"
                              "<li><strong>Right after what was done in step 1,</strong> make a line identifying the total cross section starting "
                              "with <code>//</code>.</li><li><strong>Write the total cross section data.</strong></li>"
                              "<li><strong>Create the scattering matrix</strong> considering that for each degree of Legendre, you will have a "
                              "g\' x g matrix where g is the number of energy groups.</li></ol>";

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

    fileContent.erase(std::remove(fileContent.begin(), fileContent.end(), '\r'), fileContent.end());

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

    crossSectionDataFileInfomation.numberOfEnergyGroup = findNumberInNextLine(str, beginPattern);

    if (referenceEnergyGroup > crossSectionDataFileInfomation.numberOfEnergyGroup)
    {
        eError |= ParseErrors::eNumberOfGroupDoesNotMatch;
    }

    crossSectionDataFileInfomation.numberOfLegendre = findLegenderOrder(str);

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

int ParseFile::findNumberInNextLine(std::string &input, std::regex beginPattern)
{
    std::smatch match;
    int count = 0;

    if (std::regex_search(input, match, beginPattern))
    {
        std::string::const_iterator headerEnd = match.suffix().first;
        std::string::const_iterator nextLineStart = headerEnd;

        // Move to start of next line
        nextLineStart = std::find(nextLineStart, input.cend(), '\n');
        if (nextLineStart == input.cend())
            return count;

        ++nextLineStart;

        // Find end of the next line
        std::string::const_iterator nextLineEnd = std::find(nextLineStart, input.cend(), '\n');
        std::string line(nextLineStart, nextLineEnd);

        // Count numeric values in the line
        std::regex numberPattern(R"([-+]?\b\d*\.?\d+(?:[eE][-+]?\d+)?)");
        std::sregex_iterator it(line.begin(), line.end(), numberPattern);
        std::sregex_iterator end;

        count = std::distance(it, end);
    }

    return count;
}


int ParseFile::findLegenderOrder(const std::string &input)
{
    std::regex pattern(R"(//\s*Sigma\s+Espalhamento\s+(\d+))");
    std::sregex_iterator it(input.begin(), input.end(), pattern);
    std::sregex_iterator end;

    int maxOrder = -1;

    while (it != end)
    {
        int order = std::stoi((*it)[1].str());
        if (order > maxOrder)
            maxOrder = order;
        ++it;
    }

    return maxOrder;
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

