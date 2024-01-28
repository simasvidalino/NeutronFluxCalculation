#ifndef NJOYCROSSSECTIONPDFREADER_H
#define NJOYCROSSSECTIONPDFREADER_H

#include <QString>
#include <QFile>


class NJOYCrossSectionPDFReader
{
public:
    NJOYCrossSectionPDFReader();

    QString getFileName() const;
    void setFileName(const QString &newFileName);

    std::map<int, std::vector<double> > getSigmas() const;

private:
    void readPDF();

    void processLine(QFile &file, int matNumber);

    QString fileName;

    std::map<int , std::vector<double>> sigmas;
};

#endif // NJOYCROSSSECTIONPDFREADER_H
