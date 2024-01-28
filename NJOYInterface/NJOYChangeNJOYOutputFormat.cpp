#include "NJOYChangeNJOYOutputFormat.h"

#include <QFile>
#include <QDebug>
#include <algorithm>

NJOYChangeNJOYOutputFormat::NJOYChangeNJOYOutputFormat() : oldCrossSection(-1)
{
    mTotal.reserve(30);

    std::fill(mTotal.begin(), mTotal.end(), 0.0);
}

NJOYChangeNJOYOutputFormat::~NJOYChangeNJOYOutputFormat()
{

}

void NJOYChangeNJOYOutputFormat::read(const QString fileName, const QString material)
{
    QStringList text;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qInfo()<<"File Error";
        return;
    }

    //Read header
    QByteArray line = file.readLine();
    line = file.readLine();

    //Read temperature
    line = file.readLine();
    text = transformTextToList(line);
    temperature = changeDoubleNotation(text[0]);

    //Read Energy Groups
    line = file.readLine();
    text = transformTextToList(line);

    //The two initial values are not a group
    text.pop_front();
    text.pop_front();

    while (text.at(text.size() - 2) == "1451")
    {
        for(auto index = 0; index < text.size() - 3; ++index)
        {
            energyGroups.push_back(changeDoubleNotation(text[index]));
        }

        line = file.readLine();
        text = transformTextToList(line);
    }

    //the last value is zero
    energyGroups.pop_back();

    while (!file.atEnd())
    {
        line = file.readLine();

        text = transformTextToList(line);

        if (text.size() < 4)
            continue;

        auto mat = text.at(text.size() - 4);
        auto mf  = text.at(text.size() - 3);
        auto mt  = text.at(text.size() - 2);

        if ((!mat.contains(material) )
                || ((changeDoubleNotation(mat[0]) == temperature)
                && (changeDoubleNotation(mat[1]) == 0)) )
            continue;

        if (mf == "6")
        {
            processScatteringCrossSection(text);
        }
        else if ((mf == "3") && (mt == "1"))
        {
            processTotalCrossSection(text);
        }
        else if ((mf == "3") && (mt != "1"))
        {
            processOtherCrossSection(text);
        }
        else
        {
            //qInfo()<<"Error at line "<<line;
        }

    }
}

std::vector<double> NJOYChangeNJOYOutputFormat::getTotalInDescendingOrder()
{
    std::vector<double*> ptrNumbers;

    for (auto& number : mTotal) {
        ptrNumbers.push_back(new double (number));
    }

    mTotal.clear();

    std::reverse(ptrNumbers.begin(), ptrNumbers.end());

    for (auto& number : ptrNumbers) {
        mTotal.push_back(*number);
        delete number;
    }

    return mTotal;
}

std::vector<double> NJOYChangeNJOYOutputFormat::getTotalInAscendingOrder() const
{
    return mTotal;
}

double NJOYChangeNJOYOutputFormat::changeDoubleNotation(QString str)
{
    double sigmaValue;
    auto isANumber = false;

    if (str.contains("+"))
    {
        sigmaValue = str.replace("+", "E+").toDouble(&isANumber);
    }
    else if (str.contains("-"))
    {
        sigmaValue = str.replace("-", "E-").toDouble(&isANumber);
    }
    else
    {
        sigmaValue = 0;
    }

    if (!isANumber)
        qInfo()<<"Error: bad total cross section value conversion in changeDoubleNotation function";


    return sigmaValue;
}

void NJOYChangeNJOYOutputFormat::processTotalCrossSection(const QStringList list)
{
    auto crossSectionStr = list[1];
    auto value = changeDoubleNotation(crossSectionStr);

    mTotal.push_back(value);
}

void NJOYChangeNJOYOutputFormat::processOtherCrossSection(const QStringList list)
{
    bool ok = false;
    std::vector<double> crossSection;
    auto crossSectionStr = list[1];
    auto value = changeDoubleNotation(crossSectionStr);

    int newCrossSection = list[list.size() -2].toInt(&ok);

    if (!ok)
        qInfo()<<"Error: bad total cross section value conversion in processScatteringCrossSection function";

    crossSection.push_back(value);

    allCrossSection[newCrossSection] = crossSection;

    oldCrossSection = newCrossSection;
}

void NJOYChangeNJOYOutputFormat::processScatteringCrossSection(const QStringList list)
{


}

QStringList NJOYChangeNJOYOutputFormat::transformTextToList(QByteArray &line)
{
    auto text = QString::fromUtf8(line).split(" ");
    text.removeAll(" ");
    text.removeAll("");
    text.removeDuplicates();

    return text;
}

