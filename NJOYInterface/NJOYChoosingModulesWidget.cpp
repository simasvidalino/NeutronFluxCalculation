#include "NJOYChoosingModulesWidget.h"
#include "ui_NJOYChoosingModulesWidget.h"

#include <iostream>
#include <cassert>
#include <QDir>
#include <QInputDialog>
#include <QFile>
#include <QMessageBox>

#include "NJOYChoosingModulesJsonIO.h"
#include "NJOYProjectJsonIO.h"

#include "NJOYBroadrInputDlg.h"
#include "NJOYModerInputDlg.h"
#include "NJOYGrouprInputDlg.h"
#include "NJOYReconrInputDlg.h"
#include "NJOYUnresrInputDlg.h"
#include "NJOYInterfaceDefinitions.h"

#include "NJOYStackWidget.h"

NJOYChoosingModulesWidget::NJOYChoosingModulesWidget(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::NJOYChoosingModulesWidget)
{
    ui->setupUi(this);

    setButtonGroup();

    setConnections();

    for (int id = 0; id < NJOYMOdulesQtt; ++id)
    {
        ui->buttonGroupButtons->button(id)->setEnabled(false);
    }
}

NJOYChoosingModulesWidget::~NJOYChoosingModulesWidget()
{
    delete ui;
}

std::unique_ptr<NJOYChoosingModulesJsonIO> NJOYChoosingModulesWidget::save() const
{

}

void NJOYChoosingModulesWidget::load(std::unique_ptr<NJOYChoosingModulesJsonIO> choosingModules)
{
    if (!choosingModules)
        choosingModules = std::make_unique<NJOYChoosingModulesJsonIO>();

    choosingModules = std::move(choosingModules);

    auto io       = NJOYProjectJsonIO::getInstance();
    auto generals = io->getGeneralParameters().get();
    auto tapes    = io->getChoosingIsotopes().get();

    matNumbers   = QString::fromStdString(generals->matNumber);
    reactions    = QString::fromStdString(generals->reactions);
    temperatures = QString::fromStdString(generals->temperatures);

    fileNames.clear();

    std::for_each(tapes->fileNames.begin(), tapes->fileNames.end(), [&](const auto name){
        fileNames.push_back(QString::fromStdString(name));});

    saveInformationFromOtherModule();
}

void NJOYChoosingModulesWidget::createFile(const QString sourceFile, const QString targetFile)
{
    QFile fileOrigin(sourceFile);
    QFile finalFile(targetFile);

    if (fileOrigin.exists() && fileOrigin.open(QIODevice::ReadOnly))
    {
        if (finalFile.open(QIODevice::WriteOnly))
        {
            // Copying source file to destination
            finalFile.write(fileOrigin.readAll());

            if (finalFile.error() != QFile::NoError)
                qInfo() << "Error copying file.:" << finalFile.errorString();

            finalFile.close();
        }
        else
        {
            qInfo() << "Error opening target file.:" << finalFile.errorString();
        }

        fileOrigin.close();
    }
    else
    {
        qInfo() << "Error opening target file.:" << fileOrigin.errorString();
    }
}

bool NJOYChoosingModulesWidget::findMatNumberInTape(QString isotope, QString endfFile)
{
    QString filePath = endfFile;
    bool hasWord = false;

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open tape ENDF:" << file.errorString();
        return false;
    }

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine();

        auto list = line.split(" ");
        list.removeAll("");

        //Get the antepenultimate column in ENDF file
        if (list.size() < 3)
            continue;

        auto antepenultimateColumn = list.at(list.size() -3);

        if (antepenultimateColumn == isotope)
        {
            hasWord = true;
            break;
        }

    }

    file.close();

    return hasWord;
}

void NJOYChoosingModulesWidget::openBroadrEnterData()
{
    NJOYStackWidget stack(this);
    auto io = NJOYProjectJsonIO::getInstance();

    if (!io->loadProject(jsonFormat))
        qInfo()<<"Groupr load project error";


    int matQuantity =  matNumberList.size();

    for (int index = 0; index < matQuantity; ++index)
    {
        NJOYBroadrInputDlg* dlg = new NJOYBroadrInputDlg();

        stack.addWidget(dlg, matNumberList.at(index)) ;

        dlg->load();
    }

    stack.setCurrentIndex(0);

    if (!stack.exec())
        return;
}

void NJOYChoosingModulesWidget::openGrouprEnterData()
{
    NJOYStackWidget stack(this);
    auto io = NJOYProjectJsonIO::getInstance();

    if (!io->loadProject(jsonFormat))
        qInfo()<<"Groupr load project error";


    int matQuantity =  matNumberList.size();

    for (int index = 0; index < matQuantity; ++index)
    {
        NJOYGrouprInputDlg* dlg = new NJOYGrouprInputDlg();

        stack.addWidget(dlg, matNumberList.at(index)) ;

        dlg->load();
    }

    stack.setCurrentIndex(0);

    if (!stack.exec())
        return;
}

void NJOYChoosingModulesWidget::openModerEnterData()
{
    NJOYModerInputDlg dlg(this);

    auto io = NJOYProjectJsonIO::getInstance();
    //TBD
    io->loadProject(jsonFormat);
    auto &cm = io->getChoosingModulesObj();
    auto moder = cm.getModer();

    dlg.load(std::move(moder));

    if (!dlg.exec())
        return;

    cm.setModer(dlg.save());
    io->saveProject(jsonFormat);
}

void NJOYChoosingModulesWidget::openReconrEnterData()
{
    NJOYStackWidget stack(this);
    auto io = NJOYProjectJsonIO::getInstance();
    //TBD
    io->loadProject(jsonFormat);

    auto &cm = io->getChoosingModulesObj();
    auto reconrVector = cm.getReconr();
    int matQuantity   =  matNumberList.size();

    if (reconrVector.empty())
    {
        for (int i = 0; i < matQuantity; ++i)
            reconrVector.push_back(std::make_unique<NJOYReconrInput>());
    }

    for (int index = 0; index < matQuantity; ++index)
    {
        NJOYReconrInputDlg* dlg = new NJOYReconrInputDlg();

        stack.addWidget(dlg, matNumberList.at(index)) ;
        dlg->load(std::move(reconrVector.at(index)));
    }

    stack.setCurrentIndex(0);

    if (!stack.exec())
        return;

    //save
    cm.deleteReconr();
    for (int index = 0; index < matQuantity; ++index)
    {
        auto widget = stack.getWidget(index);
        qInfo()<<widget<<index;
        auto dlg = dynamic_cast<NJOYReconrInputDlg*> (widget);

        if (nullptr != dlg)
        {
            std::unique_ptr<NJOYReconrInput> reconr = dlg->save();
            cm.setReconr(std::move(reconr));
        }
    }

    io->saveProject(jsonFormat);
}

void NJOYChoosingModulesWidget::openUnresEnterData()
{
    NJOYStackWidget stack(this);
    auto io = NJOYProjectJsonIO::getInstance();
    //TBD
    io->loadProject(jsonFormat);

    auto &cm = io->getChoosingModulesObj();
    std::vector<std::unique_ptr<NJOYUnresInput> > unresrVector = cm.getUnresr();
    int matQuantity   =  matNumberList.size();

    if (unresrVector.empty())
    {
        for (int i = 0; i < matQuantity; ++i)
            unresrVector.push_back(std::make_unique<NJOYUnresInput>());
    }

    for (int index = 0; index < matQuantity; ++index)
    {
        NJOYUnresrInputDlg* dlg = new NJOYUnresrInputDlg();

        stack.addWidget(dlg, matNumberList.at(index)) ;
        dlg->load(std::move(unresrVector.at(index)));
    }

    stack.setCurrentIndex(0);

    if (!stack.exec())
        return;

    //save
    cm.deleteUnresr();
    for (int index = 0; index < matQuantity; ++index)
    {
        auto widget = stack.getWidget(index);
        qInfo()<<widget<<index;
        auto dlg = dynamic_cast<NJOYUnresrInputDlg*> (widget);

        if (nullptr != dlg)
        {
            std::unique_ptr<NJOYUnresInput> unresr = dlg->save();
            cm.setUnresr(std::move(unresr));
        }
    }

    io->saveProject(jsonFormat);
}

void NJOYChoosingModulesWidget::setConnections()
{
    connect(ui->pushButtonBroadr, &QPushButton::clicked, this,
            &NJOYChoosingModulesWidget::openBroadrEnterData);

    connect(ui->pushButtonGroupr, &QPushButton::clicked, this,
            &NJOYChoosingModulesWidget::openGrouprEnterData);

    connect(ui->pushButtonModer, &QPushButton::clicked, this,
            &NJOYChoosingModulesWidget::openModerEnterData);

    connect(ui->pushButtonReconr, &QPushButton::clicked, this,
            &NJOYChoosingModulesWidget::openReconrEnterData);

    connect(ui->pushButtonUnresr, &QPushButton::clicked, this,
            &NJOYChoosingModulesWidget::openUnresEnterData);

    auto checkBoxConnection = [&](int id)
    {
        auto isEnabled = ui->buttonGroupButtons->button(id)->isEnabled();
        ui->buttonGroupButtons->button(id)->setEnabled(!isEnabled);
    };

    auto CleanAllSelection = [&]()
    {
        for (int id = 0; id < NJOYMOdulesQtt; ++id)
        {
            if (ui->buttonGroupCheckBoxes->button(id)->isChecked())
                emit ui->buttonGroupCheckBoxes->idClicked(id);

            ui->buttonGroupCheckBoxes->button(id)->setChecked(false);

        }
    };

    connect(ui->buttonGroupCheckBoxes, &QButtonGroup::idClicked, ui->buttonGroupButtons,
            checkBoxConnection);

    connect(ui->pushButtonCleanAllSelection, &QPushButton::clicked, this, CleanAllSelection);
}

void NJOYChoosingModulesWidget::saveInformationFromOtherModule()
{
    int tapeNumber = 20;
    auto moder = std::make_unique<NJOYModerInput>();
    std::map<std::string, std::string> inputTapesAndMatFinal;

    //Create NJOY input tapes with names sorted by mat number
    auto fileNamesResult = sortModerTapes();

    if (fileNamesResult.isEmpty())
        return;

    //Save tapes
    int tape = tapeNumber;
    auto matNumberList = matNumbers.split(";");
    auto it = matNumberList.begin();

    for (const auto &fileName : fileNamesResult)
    {
        QString finalNJOYTape = QDir::currentPath() + QString("tape")
                + QString::number(tape);

        finalNJOYTape = finalNJOYTape.remove("build");

        qInfo()<<finalNJOYTape;

        //Copy the endf tapes
        createFile(fileName, finalNJOYTape);

        //create a map with the tape and mat number
        inputTapesAndMatFinal[std::to_string(tape)] = it->toStdString() ;

        ++tape;
        ++it;
    }

    //Save moder informations
    if (fileNamesResult.size() > 1)
    {
        std::map<std::string, std::string> map;

        moder->input  = std::to_string(1);
        moder->output = std::to_string(tape);
        moder->inputTapesAndMat = inputTapesAndMatFinal;
    }
    else
    {
        moder->input  = std::to_string(tapeNumber);
        moder->output = std::to_string(tapeNumber + 1);
        moder->inputTapesAndMat.reset();
    }

    auto io = NJOYProjectJsonIO::getInstance();
    auto chm = io->getChoosingModules();

    chm->setModer(std::move(moder));
    io->setChoosingModules(std::move(chm));
}

QStringList NJOYChoosingModulesWidget::sortModerTapes()
{
    matNumberList = matNumbers.split(";");
    QStringList fileNamesResult;
    QStringList fileNames = this->fileNames;

    //Treat any wrong input
    matNumberList.removeAll("");
    matNumberList.removeDuplicates();

    //Sort mat number list
    std::vector<int> numericValues;
    numericValues.reserve(matNumberList.size());

    for (const QString& str : matNumberList) {
        int value = str.toInt();
        numericValues.push_back(value);
    }

    // Ordenar o std::vector de números em ordem inversa
    std::sort(numericValues.begin(), numericValues.end());

    // Converter o std::vector de volta para QStringList
    matNumberList.clear();
    matNumberList.reserve(numericValues.size());

    for (int value : numericValues)
    {
        matNumberList.append(QString::number(value));
    }

    matNumbers.clear();


    for (auto matIt = matNumberList.begin(); matIt != matNumberList.end(); )
    {
        bool hasMat = false;

        for (auto fileIt = fileNames.begin(); fileIt != fileNames.end(); ++fileIt)
        {
            auto path = *fileIt;

            hasMat = findMatNumberInTape(*matIt, path);

            if (hasMat)
            {
                fileNamesResult.push_back(path);

                //Update Mat number list
                matNumbers.push_back(*matIt + ";");

                ++matIt;

                break;
            }
        };

        //Unselect all modules if any mat has no ENDF tape
                if (false == hasMat)
                {
                    QMessageBox::warning(this, "Mat Number", QString("Mat number didn't find in "
                                                                     "any tape: ") + *matIt);

                    emit ui->pushButtonCleanAllSelection->clicked(true);

                    ui->pushButtonCleanAllSelection->setEnabled(false);

//                    for (const auto button : ui->buttonGroupCheckBoxes->buttons())
//                        button->setEnabled(false);

                    break;
                }
                else
                {
                    ui->pushButtonCleanAllSelection->setEnabled(true);

//                    for (const auto button : ui->buttonGroupCheckBoxes->buttons())
//                        button->setEnabled(true);
                }
    }

    qInfo()<<fileNamesResult;

    return fileNamesResult;
}

void NJOYChoosingModulesWidget::initializePage()
{
    auto io = NJOYProjectJsonIO::getInstance();

    load(io->getChoosingModules());
}

bool NJOYChoosingModulesWidget::validatePage()
{

}
bool NJOYChoosingModulesWidget::isComplete() const
{
    return true;
}

//void NJOYChoosingModulesWidget::initializePage()
//{
//    auto io       = NJOYProjectJsonIO::getInstance();
//    auto generals = io->getGeneralParameters();
//    auto tapes    = io->getChoosingIsotopes();

//    if (!generals)
//        generals = std::make_unique<NJOYGeneralParametersInput>();

//    if (!tapes)
//        tapes = std::make_unique<NJOYChoosingIsotopes>();

//    matNumbers   = QString::fromStdString(generals->matNumber);
//    reactions    = QString::fromStdString(generals->reactions);
//    temperatures = QString::fromStdString(generals->temperatures);

//    fileNames.clear();

//    std::for_each(tapes->fileNames.begin(), tapes->fileNames.end(), [&](const auto name){
//        fileNames.push_back(QString::fromStdString(name));});

//    saveModerInformationFromOtherModule();
//}

void NJOYChoosingModulesWidget::setButtonGroup()
{
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxAcer, Acer);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxBroadr, Broadr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxCcccr, Ccccr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxCovr, Covr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxDtfr, Dtfr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxError, Errorr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxGaminr, Gaminar);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxGaspr, Gaspr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxGroupr, Groupr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxLeapr, Leapr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxHeatr, Heatr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxMatxsr, Matxsr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxModer, Moder);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxMixr, Mixr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxPlotr, Plotr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxPowr, Powr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxPurr, Purr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxReconr, Reconr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxResxsr, Resxsr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxThermr, Thermr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxUnresr, Unresr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxViewr, Viewr);
    ui->buttonGroupCheckBoxes->setId(ui->checkBoxWimsr, Wimsr);

    ui->buttonGroupButtons->setId(ui->pushButtonAcer, Acer);
    ui->buttonGroupButtons->setId(ui->pushButtonBroadr, Broadr);
    ui->buttonGroupButtons->setId(ui->pushButtonCcccr, Ccccr);
    ui->buttonGroupButtons->setId(ui->pushButtonCovr, Covr);
    ui->buttonGroupButtons->setId(ui->pushButtonDtfr, Dtfr);
    ui->buttonGroupButtons->setId(ui->pushButtonError, Errorr);
    ui->buttonGroupButtons->setId(ui->pushButtonGaminr, Gaminar);
    ui->buttonGroupButtons->setId(ui->pushButtonGaspr, Gaspr);
    ui->buttonGroupButtons->setId(ui->pushButtonGroupr, Groupr);
    ui->buttonGroupButtons->setId(ui->pushButtonHeatr, Heatr);
    ui->buttonGroupButtons->setId(ui->pushButtonLeapr, Leapr);
    ui->buttonGroupButtons->setId(ui->pushButtonMatxsr, Matxsr);
    ui->buttonGroupButtons->setId(ui->pushButtonModer, Moder);
    ui->buttonGroupButtons->setId(ui->pushButtonMixr, Mixr);
    ui->buttonGroupButtons->setId(ui->pushButtonPlotr, Plotr);
    ui->buttonGroupButtons->setId(ui->pushButtonPowr, Powr);
    ui->buttonGroupButtons->setId(ui->pushButtonPurr, Purr);
    ui->buttonGroupButtons->setId(ui->pushButtonReconr, Reconr);
    ui->buttonGroupButtons->setId(ui->pushButtonResxsr, Resxsr);
    ui->buttonGroupButtons->setId(ui->pushButtonThermr, Thermr);
    ui->buttonGroupButtons->setId(ui->pushButtonUnresr, Unresr);
    ui->buttonGroupButtons->setId(ui->pushButtonViewr, Viewr);
    ui->buttonGroupButtons->setId(ui->pushButtonWimsr, Wimsr);
}

template<typename T>
std::vector<std::unique_ptr<T> > NJOYChoosingModulesWidget::fillWithDefault(T classObj, int numberOfData)
{
    std::vector<std::unique_ptr<T>> vec;

    for (int i = 0; i < numberOfData; ++i)
    {
        auto ptr = std::make_unique<T>();
        vec.push_back(ptr);
    }

    return vec;
}


