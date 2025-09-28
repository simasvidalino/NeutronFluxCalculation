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
    referenceLegendreOrder  = legendreOrder;
    referenceNumberOfZones  = numberOfZones;
}

std::string ParseFile::makeInstruction()
{
    std::string instruction =
        "<p><strong>To create a valid cross-section data file, follow the rules below:</strong></p>"
        "<ol style='margin-top:8px; margin-bottom:8px;'>"

        "<li style='margin-bottom:12px;'><strong>Mark the beginning of a material zone:</strong><br>"
        "Start the line with <code>///</code> followed by the zone identifier "
        "(for example, <code>/// Zone 1</code>).</li>"

        "<li style='margin-bottom:12px;'><strong>Define the total cross section:</strong><br>"
        "Add a line starting with <code>//</code>, such as "
        "<code>// Total Cross Section g</code>, and then provide the numerical data for "
        "each energy group <code>g</code>.</li>"

        "<li style='margin-bottom:12px;'><strong>Write the total cross section values:</strong><br>"
        "Provide one value per energy group in the order defined above.</li>"

        "<li style='margin-bottom:12px;'><strong>Define the scattering matrices:</strong><br>"
        "Before writing the scattering data of each Legendre order <code>L</code>, "
        "insert a line starting with <code>// Scattering Cross Section L</code>. "
        "Here, <code>L</code> is the degree of the Legendre expansion "
        "(e.g., <code>L = 0</code>, <code>L = 1</code>, etc.).</li>"

        "<li><strong>Provide the scattering matrix values:</strong><br>"
        "For each order <code>L</code>, write a matrix of dimensions <code>g × g′</code>, "
        "where <code>g′</code> represents the <em>initial</em> (incident) energy group (columns) "
        "and <code>g</code> represents the <em>final</em> (outgoing) energy group (rows).<br>"
        "Thus, each entry corresponds to scattering from group <code>g′</code> into group <code>g</code>.</li>"

        "</ol>";

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

std::string ParseFile::makeExample()
{
    std::string example;

    for (int zone = 1; zone <= referenceNumberOfZones; ++zone)
    {
        example += "///Zone " + std::to_string(zone) + "\n";
        example += "//Total Cross Section g (g=1.." + std::to_string(referenceEnergyGroup) + ")\n";
        for (int g = 0; g < referenceEnergyGroup; ++g)
        {
            example += "0.0 ";
        }

        example += "\n";

        for (int l = 0; l <= referenceLegendreOrder; ++l)
        {
            example += "//Scattering Cross Section " + std::to_string(l) + " g'g (g row; g' colunm)\n";
            for (int g = 0; g < referenceEnergyGroup; ++g)
            {
                for (int gline = 0; gline < referenceEnergyGroup; ++gline)
                {
                    example += "0.0 ";
                }

                example += "\n";
            }
        }
    }

    return example;
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
    if (true == bSkipParse)
    {
        crossSectionDataFileInfomation = { referenceEnergyGroup, referenceNumberOfZones, referenceLegendreOrder };
        return ParseErrors::eParseDisabled;
    }

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

    if (referenceLegendreOrder > crossSectionDataFileInfomation.numberOfLegendre)
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
    std::regex pattern(R"(//\s*(?:Sigma\s+Espalhamento|Scattering\s+Cross\s+Section)\s+(\d+))");

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
    : bSkipParse(false)
{

}

ParseFile::~ParseFile()
{

}

void ParseFile::setBSkipParse(bool newBSkipParse)
{
    bSkipParse = newBSkipParse;
}

bool ParseFile::getBSkipParse() const
{
    return bSkipParse;
}

CrossSectionDataFilerParameters& ParseFile::getCrossSectionDataFileInfomation()
{
    return crossSectionDataFileInfomation;
}

