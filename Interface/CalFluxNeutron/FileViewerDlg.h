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

    QString getFilePath() const;

    void loadTxtFile(std::string &file);

    void makeReadOnly();

    void readHTMFile(std::string filePath); //read the txt

    void setDefaultFileName(const QString& name);

    ParseFile::ParseErrors parseFile();

protected:
    virtual void accept() override;
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

    QString filePath;
    QString defaultFileName;

    void showFindDialog();

    // QWidget interface
protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
};
