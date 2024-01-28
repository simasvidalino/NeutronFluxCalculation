#pragma once

#include <QString>
#include <vector>
#include <map>

class NJOYChangeNJOYOutputFormat
{
public:
    NJOYChangeNJOYOutputFormat();
    ~NJOYChangeNJOYOutputFormat();


    void read(const QString fileName, const QString material);

    //The firts group is the most energetic
    std::vector<double> getTotalInDescendingOrder();
    std::vector<double> getTotalInAscendingOrder() const;


private:

    double changeDoubleNotation(QString str);
    void processTotalCrossSection(const QStringList list);
    void processOtherCrossSection(const QStringList list);
    void processScatteringCrossSection(const QStringList list);

    QStringList transformTextToList(QByteArray &line);

    double temperature;
    int oldCrossSection;

    std::vector<double> energyGroups;
    std::vector<double> mTotal;
    std::map<int, std::vector<double>> allCrossSection;

};
