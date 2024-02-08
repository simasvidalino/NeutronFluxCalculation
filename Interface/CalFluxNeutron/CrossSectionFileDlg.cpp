#include "CrossSectionFileDlg.h"
#include "ui_CrossSectionFileDlg.h"

#include "ParseFile.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTableView>
#include <QTableWidgetItem>

#include <iostream>

CrossSectionFileDlg::CrossSectionFileDlg(QWidget *parent,
                                         QStringList materials,
                                         int energyGroup,
                                         int legendreOrder) :
    QDialog(parent),
    energyGroup(energyGroup),
    legendreOrder(legendreOrder),
    materialList(materials),
    ui(new Ui::CrossSectionFileDlg)
{
    ui->setupUi(this);

    initDlg();
}

CrossSectionFileDlg::~CrossSectionFileDlg()
{
    delete ui;
}

void CrossSectionFileDlg::accept()
{
    parseFile();

    if (eParseError == ParseFile::ParseErrors::eOk)
    {
        QMessageBox::information(this, "Parser Information",
                                 "Simple analysis passed");
        QDialog::accept();
    }
    else
        QMessageBox::information(this, "Parser Error", "Project data and material data do not match.");
}

void CrossSectionFileDlg::clearText()
{
    ui->textEdit->clear();
}
void CrossSectionFileDlg::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Text Files (*.txt)");

    if (!fileName.isEmpty())
    {
        readFile(fileName);
    }
}

void CrossSectionFileDlg::parseFile()
{
    auto fileContent = ui->textEdit->toPlainText().toStdString();

    ParseFile::getInstance()->setProjectData(energyGroup, legendreOrder, materialList.size());

    eParseError = ParseFile::getInstance()->parseString(fileContent);

    qWarning()<<"CrossSectionFileDlg::parseFile()"<<eParseError;
}

void CrossSectionFileDlg::saveText()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "File Text(txt) (*.txt)");
    const QString instruction = "<p><strong>To create a valid text format, follow the rules below:</strong></p> <ol>"
                                "<li><strong>Before the numerical data for material zone,</strong> start the line with <code>///</code>.</li>"
                                "<li><strong>Right after what was done in step 1,</strong> make a line identifying the total cross section starting "
                                "with <code>//</code>.</li><li><strong>Write the total cross section data.</strong></li>"
                                "<li><strong>Create the scattering matrix</strong> considering that for each degree of Legendre, you will have a "
                                "g x g matrix where g is the number of energy groups.</li></p>";

    if (!fileName.isEmpty())
    {
        parseFile();

        if (eParseError != ParseFile::eOk)
        {
            QMessageBox::information(this, "Parser Error", instruction);
            return;
        }

        pathCrossSection = fileName;

        writeFile(fileName);
    }
}

void CrossSectionFileDlg::readFile(QString &filePath)
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

void CrossSectionFileDlg::writeFile(QString &filePath)
{
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

void CrossSectionFileDlg::initDlg()
{
    setConnection();
}

void CrossSectionFileDlg::setConnection()
{
    connect(ui->commandLinkButtonOpen, &QCommandLinkButton::clicked, this, &CrossSectionFileDlg::openFile);
    connect(ui->commandLinkButtonClear, &QCommandLinkButton::clicked, this, &CrossSectionFileDlg::clearText);
    connect(ui->commandLinkButtonSave, &QCommandLinkButton::clicked, this, &CrossSectionFileDlg::saveText);
    connect(ui->commandLinkButtonParse, &QCommandLinkButton::clicked, this, &CrossSectionFileDlg::parseFile);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CrossSectionFileDlg::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &CrossSectionFileDlg::reject);
}

void CrossSectionFileDlg::setPathCrossSection(const QString &newPathCrossSection)
{
    pathCrossSection = newPathCrossSection;

    if (!pathCrossSection.isEmpty())
        readFile(pathCrossSection);
}

QString CrossSectionFileDlg::getPathCrossSection() const
{
    return pathCrossSection;
}

ParseFile::ParseErrors CrossSectionFileDlg::getEParseError() const
{
    return eParseError;
}
