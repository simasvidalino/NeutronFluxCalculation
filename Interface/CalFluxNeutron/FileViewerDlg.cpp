#include "FileViewerDlg.h"
#include "ui_FileViewerDlg.h"

#include "ParseFile.h"

#include <QKeyEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDesktopServices>
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
    saveText();
    QDialog::accept();
}

void FileViewerDlg::clearText()
{
    ui->textEdit->clear();
}
void FileViewerDlg::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Text Files (*.txt)");

    this->filePath = fileName;

    readFile(fileName);
}

 ParseFile::ParseErrors FileViewerDlg::parseFile()
{
    auto fileContent = ui->textEdit->toPlainText().toStdString();

    ParseFile::getInstance()->setProjectData(energyGroup, legendreOrder, zoneNumber);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Project data information.");
    msgBox.setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint );
    msgBox.setIcon(QMessageBox::Information);
    msgBox.addButton(QMessageBox::Ok);

    eParseError = ParseFile::getInstance()->parseString(fileContent);

    if (eParseError == ParseFile::ParseErrors::eOk)
    {
        msgBox.setText("Parser Information: Simple analysis passed ");
        msgBox.exec();
    }
    else
    {
        QPushButton *exampleButton = msgBox.addButton("Generate Example", QMessageBox::ActionRole);

        msgBox.setText("<p>Project data and material data do not match.</p>");
        msgBox.setInformativeText(ParseFile::getInstance()->makeInstruction().c_str());
        msgBox.exec();

        if (msgBox.clickedButton() == exampleButton)
        {
            QString exampleText = QString::fromStdString(ParseFile::getInstance()->makeExample());

            ui->textEdit->setText(exampleText);
        }
    }

    return eParseError;
}

void FileViewerDlg::saveText()
{
    writeFile(filePath);
}

void FileViewerDlg::saveTextDlg()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::homePath() + "/" + defaultFileName, "Text Files (*.txt)");

    if (fileName.isEmpty())
        return;

    if (!fileName.contains(".txt"))
    {
        fileName.push_back(".txt");
    }

    this->filePath = fileName;

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

void FileViewerDlg::readHTMFile(std::string filePath)
{
    QFile file(QString::fromStdString(filePath));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    ui->textEdit->setHtml(QString::fromUtf8(file.readAll()));
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
    QObject::connect(ui->commandLinkButtonOpen, &QCommandLinkButton::clicked, this, &FileViewerDlg::openFile);
    QObject::connect(ui->commandLinkButtonClear, &QCommandLinkButton::clicked, this, &FileViewerDlg::clearText);
    QObject::connect(ui->commandLinkButtonSave, &QCommandLinkButton::clicked, this, &FileViewerDlg::saveTextDlg);
    QObject::connect(ui->commandLinkButtonParse, &QCommandLinkButton::clicked, this, &FileViewerDlg::parseFile);

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &FileViewerDlg::accept);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &FileViewerDlg::reject);
}

void FileViewerDlg::keyPressEvent(QKeyEvent *event)
{
    if (    (event->modifiers() & Qt::ControlModifier)
         && (event->key() == Qt::Key_F)
         && (!ui->textEdit->isReadOnly())) //it works only fo
    {
        showFindDialog();
    }
    else
    {
        QDialog::keyPressEvent(event);
    }
}

void FileViewerDlg::loadTxtFile(std::string &file)
{
    filePath = QString::fromStdString(file);
    readFile(filePath);

    ui->textEdit->moveCursor(QTextCursor::Start);
}

void FileViewerDlg::makeReadOnly()
{
    ui->groupBox->hide();
    ui->buttonBox->setVisible(false);
    setWindowTitle(filePath);
    ui->textEdit->setReadOnly(true);
}

void FileViewerDlg::setDefaultFileName(const QString &name)
{
    QString baseName = QFileInfo(name).completeBaseName();

    baseName.append("_Material");

    defaultFileName = baseName;
}

QString FileViewerDlg::getFilePath() const
{
    return filePath;
}

ParseFile::ParseErrors FileViewerDlg::getEParseError() const
{
    return eParseError;
}

void FileViewerDlg::showFindDialog()
{
    bool ok      = false;
    QString text = QInputDialog::getText(this, tr("Find"), tr("Word to find:"), QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty())
    {
        ui->textEdit->setFocus();

        QRegularExpression rx(QRegularExpression::escape(text));
        rx.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        bool found = ui->textEdit->find(rx);

        if (!found)
        {
            QMessageBox::information(this, tr("Find"), tr("No occurrence of \"%1\" found.").arg(text));
        }
    }
}
