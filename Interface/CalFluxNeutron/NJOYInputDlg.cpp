#include "NJOYInputDlg.h"
#include "ui_NJOYInputDlg.h"

#include "InterFaceDefinitions.h"

#include <fstream>
#include <iostream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QProcess>

NJOYInputDlg::NJOYInputDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NJOYInputDlg)
{
    ui->setupUi(this);

    setConnections();

    initDlg();
}

NJOYInputDlg::~NJOYInputDlg()
{
    delete ui;
}

void NJOYInputDlg::onHelpRequested()
{
    QUrl fileUrl = QUrl::fromLocalFile(":/PDFs/Resources/njoy16.pdf");

    bool success = QDesktopServices::openUrl(fileUrl);

    if (!success)
    {
        qWarning()<<QString("Failed to open NJOY Manual ") + fileUrl.errorString();
    }
}

void NJOYInputDlg::onRunNJOY()
{
    //Save the output file name
    QString filePath = QFileDialog::getSaveFileName(nullptr, "Save", "", "Text File (*.txt)");

    if (!filePath.contains(".txt"))
        filePath.push_back(".txt");

    if (filePath.isEmpty())
        return;

    saveInputFile();

    QProcess process(this);
    QString njoyExecutable;
    QStringList arguments;

    QString appDirPath = QCoreApplication::applicationDirPath();
    njoyExecutable = appDirPath + "/bin/njoy21";
    arguments << "-i" << "Input.txt" << "-o" << filePath ;

    process.start(njoyExecutable, arguments);

    if (!process.waitForFinished())
        qInfo() << "Make failed: " << process.errorString();
    else
        qInfo() << "Make output: " << process.readAll();

    auto output = process.readAllStandardOutput();
    qInfo() << output;
}

void NJOYInputDlg::initDlg()
{
    ui->labelLink->setText(Interface::getIAEAAdress());
    ui->labelLink->setToolTip("https://www-nds.iaea.org/exfor/endf.htm");
    ui->labelLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->labelLink->setOpenExternalLinks(true);
}

void NJOYInputDlg::setConnections()
{
    connect(ui->pushButtonExecute, &QPushButton::clicked, this, &NJOYInputDlg::onRunNJOY);
    connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &NJOYInputDlg::onHelpRequested);
}

void NJOYInputDlg::saveInputFile()
{
    std::ofstream outFile("Input.txt");

    if (!outFile.is_open())
    {
        qWarning() << "Error opening file: ";
        return;
    }

    outFile << ui->textEdit->toPlainText().toStdString() << std::endl;
    outFile.close();
}
