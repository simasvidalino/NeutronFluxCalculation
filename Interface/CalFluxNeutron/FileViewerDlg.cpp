#include "FileViewerDlg.h"
#include "ui_FileViewerDlg.h"

#include "ParseFile.h"

#include <QFileDialog>
#include <QMessageBox>

#include <iostream>

FileViewerDlg::FileViewerDlg(QWidget *parent,
                             int izoneNumber,
                             int ienergyGroup,
                             int ilegendreOrder) :
    QDialog(parent),
    energyGroup(ienergyGroup),
    legendreOrder(ilegendreOrder),
    zoneNumber(izoneNumber),
    ui(new Ui::FileViewerDlg)
{
    ui->setupUi(this);

    initDlg();
}

FileViewerDlg::~FileViewerDlg()
{
    delete ui;
}

void FileViewerDlg::accept()
{
    if (saveText())
    {
        QDialog::accept();
    }
}

void FileViewerDlg::clearText()
{
    ui->textEdit->clear();
}
void FileViewerDlg::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Text Files (*.txt)");

    pathCrossSection = fileName;

    readFile(fileName);
}

 ParseFile::ParseErrors FileViewerDlg::parseFile()
{
    auto fileContent = ui->textEdit->toPlainText().toStdString();

    ParseFile::getInstance()->setProjectData(energyGroup, legendreOrder, zoneNumber);

    eParseError = ParseFile::getInstance()->parseString(fileContent);

    if (eParseError == ParseFile::ParseErrors::eOk)
    {
        qInfo("Parser Information: Simple analysis passed");
    }
    else
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Project data and material data do not match.");
        msgBox.setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint );
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(ParseFile::getInstance()->makeInstruction().c_str());
        msgBox.addButton(QMessageBox::Ok);
        msgBox.exec();
    }

    return eParseError;
}

bool FileViewerDlg::saveText()
{
    bool isSave = false;

    parseFile();

    if (eParseError == ParseFile::ParseErrors::eOk)
    {
        isSave = true;
        writeFile(pathCrossSection);
    }

    return isSave;
}

void FileViewerDlg::saveTextDlg()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "Text Files (*.txt)");

    if (fileName.isEmpty())
        return;

    if (!fileName.contains(".txt"))
    {
        fileName.push_back(".txt");
    }

    pathCrossSection = fileName;

    saveText();
}

void FileViewerDlg::readFile(QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Open file failed";
        return;
    }

    QTextStream in(&file);
    QString fileContent;

    fileContent = in.readAll();

    int index = fileContent.indexOf("///");

    if (index != -1)
    {
        QString contentAfter = fileContent.mid(index);

        ui->textEdit->setPlainText(contentAfter);
    }
    else
    {
        ui->textEdit->setPlainText(fileContent);
    }

    file.close();
}

void FileViewerDlg::writeFile(QString &filePath)
{
    //The filePath will be updated in json only when the user runs the app.
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "Error opening file for writing";
        return;
    }

    QTextStream stream(&file);
    stream << ui->textEdit->toPlainText();

    file.close();
}

void FileViewerDlg::initDlg()
{
    setConnection();
}

void FileViewerDlg::setConnection()
{
    connect(ui->commandLinkButtonOpen, &QCommandLinkButton::clicked, this, &FileViewerDlg::openFile);
    connect(ui->commandLinkButtonClear, &QCommandLinkButton::clicked, this, &FileViewerDlg::clearText);
    connect(ui->commandLinkButtonSave, &QCommandLinkButton::clicked, this, &FileViewerDlg::saveTextDlg);
    connect(ui->commandLinkButtonParse, &QCommandLinkButton::clicked, this, &FileViewerDlg::parseFile);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &FileViewerDlg::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &FileViewerDlg::reject);
}

void FileViewerDlg::loadCrossSectionFile(std::string &newPathCrossSection)
{
    pathCrossSection = QString::fromStdString(newPathCrossSection);
    readFile(pathCrossSection);
}

void FileViewerDlg::makeReadOnly()
{
    ui->groupBox->hide();
    ui->buttonBox->setVisible(false);
    setWindowTitle(pathCrossSection);
    ui->textEdit->setReadOnly(true);
}

QString FileViewerDlg::getPathCrossSection() const
{
    return pathCrossSection;
}

ParseFile::ParseErrors FileViewerDlg::getEParseError() const
{
    return eParseError;
}
