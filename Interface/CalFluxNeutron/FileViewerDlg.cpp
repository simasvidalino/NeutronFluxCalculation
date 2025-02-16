#include "FileViewerDlg.h"
#include "ui_FileViewerDlg.h"

#include "ParseFile.h"

#include <QFileDialog>
#include <QMessageBox>

#include <iostream>

FileViewerDlg::FileViewerDlg(QWidget *parent,
                                         QStringList materials,
                                         int energyGroup,
                                         int legendreOrder) :
    QDialog(parent),
    energyGroup(energyGroup),
    legendreOrder(legendreOrder),
    materialList(materials),
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
    parseFile();

    if (eParseError == ParseFile::ParseErrors::eOk)
    {
        writeFile(pathCrossSection);

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

void FileViewerDlg::parseFile()
{
    const QString instruction = "<p><strong>To create a valid text format, follow the rules below:</strong></p> <ol>"
                                "<li><strong>Before the numerical data for material zone,</strong> start the line with <code>///</code>.</li>"
                                "<li><strong>Right after what was done in step 1,</strong> make a line identifying the total cross section starting "
                                "with <code>//</code>.</li><li><strong>Write the total cross section data.</strong></li>"
                                "<li><strong>Create the scattering matrix</strong> considering that for each degree of Legendre, you will have a "
                                "g x g matrix where g is the number of energy groups.</li></p>";

    auto fileContent = ui->textEdit->toPlainText().toStdString();

    ParseFile::getInstance()->setProjectData(energyGroup, legendreOrder, materialList.size());

    eParseError = ParseFile::getInstance()->parseString(fileContent);

    if (eParseError == ParseFile::ParseErrors::eOk)
    {
        QMessageBox::information(this, "Parser Information",
                                 "Simple analysis passed");
    }
    else
    {
        QMessageBox::information(this, "Project data and material data do not match.", instruction);
    }

}

void FileViewerDlg::saveText()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "Text Files (*.txt)");

    if (!fileName.contains(".txt"))
    {
        fileName.push_back(".txt");
    }

    if (!fileName.isEmpty())
    {
        parseFile();

        pathCrossSection = fileName;

        writeFile(pathCrossSection);
    }
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
    connect(ui->commandLinkButtonSave, &QCommandLinkButton::clicked, this, &FileViewerDlg::saveText);
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
