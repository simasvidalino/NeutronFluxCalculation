#pragma once

#include <QDialog>
#include <map>

#include "ParseFile.h"

namespace Ui {
class FileViewerDlg;
}

class FileViewerDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FileViewerDlg(QWidget *parent   = nullptr,
                           int izoneNumber    = 0,
                           int ienergyGroup   = 0,
                           int ilegendreOrder = 0);
    ~FileViewerDlg();

    ParseFile::ParseErrors getEParseError() const;

    QString getPathCrossSection() const;

    void loadCrossSectionFile(std::string &newPathCrossSection);

    void makeReadOnly();

    void setDefaultFileName(const QString& name);

    ParseFile::ParseErrors parseFile();

protected:
    virtual void accept();
    void readFile(QString &filePath); //read the txt
    void saveText();
    void writeFile(QString &filePath);

private slots:
    void clearText();
    void openFile(); //use a screen to choose a file
    void saveTextDlg();

private:
    struct legendreData
    {
        int legendreNumber = 0;
        std::map<double, double> scattering;
    };

    Ui::FileViewerDlg *ui;

    void initDlg();
    void setConnection();

    const int energyGroup;
    const int legendreOrder;
    const int zoneNumber;

    ParseFile::ParseErrors eParseError;

    QString pathCrossSection;
    QString defaultFileName;

};

