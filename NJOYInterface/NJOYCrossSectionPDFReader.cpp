#include "NJOYCrossSectionPDFReader.h"

#include <QFile>
#include <QDebug>
#include <QRegularExpression>

NJOYCrossSectionPDFReader::NJOYCrossSectionPDFReader()
{
    fileName = "/home/andreiasimas/Documentos/output_N.txt";
    readPDF();
}

void NJOYCrossSectionPDFReader::readPDF()
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qInfo()<<"File Error";
        return;
    }

    const QString key1 = "mt";
    const QString key2 = "mf";

    while (!file.atEnd())
    {
        QByteArray line = file.readLine();

        QString text = QString::fromUtf8(line);

        if (text.contains(key1) && text.contains(key2))
        {
            int index = text.indexOf(key1) + 1;

            QString mtNumber = text.mid(index + key1.length(), 2).trimmed();

            processLine(file, mtNumber.toInt());
        }

    }
}

void NJOYCrossSectionPDFReader::processLine(QFile &file, int matNumber)
{
    QString text;

    file.readLine();
    file.readLine();

    while (!file.atEnd() && !text.contains("group constants"))
    {
        auto line = file.readLine();

        text = QString::fromUtf8(line).trimmed();

        auto textVect = text.split(" ");

        QRegularExpression exp("^.+$"); // expressão regular que corresponde a strings não vazias
        textVect = textVect.filter(exp); // filtra as strings que não estão vazias

        std::vector<double> crossSection;

        int i = 0;

        if (textVect.size() == 2)
        {
            bool isANumber = false;
            textVect[0].toInt(&isANumber);

            if (false == isANumber)
            {
                continue;
            }

            double sigmaValue;
            isANumber = false;

            if (textVect[1].contains("+"))
            {
                sigmaValue = textVect.back().replace("+", "E+").toDouble(&isANumber);
            }
            else if (textVect[1].contains("-"))
            {
                sigmaValue = textVect.back().replace("-", "E-").toDouble(&isANumber);
            }
            else
            {
                qInfo()<<"Erro";
            }

            crossSection.push_back(sigmaValue);

            qInfo()<<crossSection[i];
            ++i;
        }

        sigmas[matNumber] = crossSection;
    }

}

std::map<int, std::vector<double> > NJOYCrossSectionPDFReader::getSigmas() const
{
    return sigmas;
}

QString NJOYCrossSectionPDFReader::getFileName() const
{
    return fileName;
}

void NJOYCrossSectionPDFReader::setFileName(const QString &newFileName)
{
    fileName = newFileName;
}
