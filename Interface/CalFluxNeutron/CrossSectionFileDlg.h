#pragma once

#include <QDialog>
#include <map>

#include "ParseFile.h"

namespace Ui {
class CrossSectionFileDlg;
}

class CrossSectionFileDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CrossSectionFileDlg(QWidget *parent   = nullptr,
                                 QStringList materials = {},
                                 int energyGroup   = 0,
                                 int legendreOrder = 0);
    ~CrossSectionFileDlg();

    ParseFile::ParseErrors getEParseError() const;

    QString getPathCrossSection() const;

    void loadCrossSectionFile(std::string &newPathCrossSection);

protected:
    virtual void accept();

    void readFile(QString &filePath); //read the txt
    void writeFile(QString &filePath);


private slots:
    void clearText();
    void openFile(); //use a screen to choose a file
    void parseFile();
    void saveText();

private:
    struct legendreData
    {
        int legendreNumber = 0;
        std::map<double, double> scattering;
    };

    Ui::CrossSectionFileDlg *ui;

    void initDlg();
    void setConnection();

    const int energyGroup;
    const int legendreOrder;

    QStringList materialList;

    ParseFile::ParseErrors eParseError;

    QString pathCrossSection;
};

