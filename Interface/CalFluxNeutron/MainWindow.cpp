#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <mutex>

#include "DataMatrices.h"
#include "FileViewerDlg.h"
#include "NeutronFlowJsonIO.h"
#include "VariablesUsed.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QLineSeries>
#include <QMessageBox>
#include <QThread>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , fileName(Interface::getDefaultProjectName())
{
    ui->setupUi(this);

    init();
}

MainWindow::~MainWindow()
{
    calculationThread->quit();
    calculationThread->wait();

    delete ui;
}

void MainWindow::calculateNeutronFluxUsingDD()
{
    if (!saveProject())
        return;

    startWork();

    if (!proj)
        proj = std::make_shared<ProjectData>();

    worker->setProjData(*proj);

    this->statusBar()->showMessage("Calculating...");

    ui->widgetRegion->setPushButtonCalculateFluxEnable(false);

    emit startProcess();
}

void MainWindow::changeFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(
        &ok, QFont("Helvetica [Cronyx]", 10), this);
    if (ok)
    {
        this->setFont(font);
    }
    else
    {
        qWarning()<<"Font issue";
    }
}

void MainWindow::changeViewMode()
{
    if (this->isFullScreen())
    {
        ui->actionScreenMode->setText("Full Screen");
        this->showNormal();
    }
    else
    {
        ui->actionScreenMode->setText("Nomal Screen");
        this->showFullScreen();
    }
}

void MainWindow::changePalette()
{
    const QString darkText = "Color scheme to Dark";

    if (ui->actionPalette->text() == darkText)
    {
        ui->actionPalette->setText("Color scheme to Light");
        QApplication::setPalette(Interface::getDarkPalette());
    }
    else
    {
        ui->actionPalette->setText(darkText);
        QApplication::setPalette(Interface::getLightPalette());
    }
}

void MainWindow::onOutputData(std::shared_ptr<CalculatedData> data)
{
    proj->totalScatteringCrossSectionFilePath =  data->matrices.scatteringCrossSectionFile;
    proj->absorptionCrossSectionFilePath      =  data->matrices.absorptionCrossSectionFile;

    proj->scalarFluxFile = data->scalarFluxFile;
    proj->absorptionRateFile = data->absorptionRateFile;
    proj->absorptionRatePerNodeFile = data->absorptionRatePerNodeFile;
    proj->averageNeutronFluxPerRegionFile = data->averageNeutronFluxPerRegionFile;

    updateFluxChart(data);
    updateFluxTable(data);
    updateAbsRateChart(data);
    updateAbsRateTable(data);

    ui->widgetNeutronAbsorpt->commitChanges();
    ui->widgetNeutronScalarFlux->commitChanges();

    if (!fileName.contains("Default"))
    {
        NeutronFlowJsonIO::getInstance()->setGeneralProjectData(proj);
        NeutronFlowJsonIO::getInstance()->saveProject(Interface::jsonFormat, fileName);
    }
    else
    {
        showDefaultProjectWarning();
    }
}

void MainWindow::openProjectFileDlg()
{
    QString filter = "JSON Files (*.json);;Text Files (*.txt)";
    auto fileStr = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(), filter);

    if (fileStr.isEmpty())
        return;

    ui->widgetRegion->setEnableGUI(true);

    fileName = fileStr;

    openProject();

    ui->tabWidget->setCurrentIndex(tabInputData);
    ui->widgetNeutronAbsorpt->clearChart();
    ui->widgetNeutronScalarFlux->clearChart();
}

void MainWindow::saveMaterialData()
{
    std::string pathSaved;

    try
    {
        std::string newPath = fileName.toStdString();
        std::string oldPath =  proj->scatteringFilePath;

        proj->scatteringFilePath = BuildMatrices::getInstance()->saveMaterialData(newPath, oldPath);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        qWarning() << "Error:" << e.what();
    }
}

void MainWindow::openProject()
{
    QString title = QString(Interface::getWindowTitle()) + QString(": ") + fileName.split("/").back();
    this->setWindowTitle(title);

    this->statusBar()->showMessage("Waite...");
    ui->widgetRegion->setPushButtonCalculateFluxEnable(false);

    NeutronFlowJsonIO::getInstance()->loadProject(Interface::jsonFormat, fileName);
    proj = NeutronFlowJsonIO::getInstance()->getGeneralProjectData();

    if (!proj)
        proj = std::make_shared<ProjectData>();

    ui->widgetNeutronAbsorpt->setPeriodicityValue(proj->periodicity);
    ui->widgetNeutronScalarFlux->setPeriodicityValue(proj->periodicity);

    ui->widgetRegion->setGeneralProjectData(proj);

    QTimer::singleShot(2000, this, [&](){
        this->statusBar()->showMessage("Ready");
        ui->widgetRegion->setPushButtonCalculateFluxEnable(true);
    });
}

void MainWindow::saveProjectFileDlg()
{
    QString filter = "JSON Files (*.json);;Text Files (*.txt)";
    auto fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), filter);
    const bool saveMaterialData = true;

    if (fileName.isEmpty())
        return;

    ui->widgetRegion->setEnableGUI(true);
    this->fileName = fileName;

    (void)saveProject(saveMaterialData);
}

void MainWindow::showDataInFile(std::string& file)
{
    if (file.empty())
        return;

    FileViewerDlg dlg(this);

    dlg.loadCrossSectionFile(file);
    dlg.makeReadOnly();
    dlg.exec();
}

bool MainWindow::saveProject(bool saveMaterialDataFile)
{
    bool projectSaved = false;

    if (fileName.isEmpty())
    {
        QMessageBox::information(this, "Project File Not Found", "Please save your project before proceeding.");
    }
    else
    {
        //Here we save dlg project, the calculated data files are calculated later
        projectSaved = true;
        proj = ui->widgetRegion->getGeneralProjectData();
        proj->periodicity = ui->widgetNeutronScalarFlux->getPeriodicityValue();

        if (saveMaterialDataFile)
            saveMaterialData();

        NeutronFlowJsonIO::getInstance()->setGeneralProjectData(proj);
        NeutronFlowJsonIO::getInstance()->saveProject(Interface::jsonFormat, fileName);

        QString title = QString(Interface::getWindowTitle()) + QString(": ") + fileName.split("/").back();
        this->setWindowTitle(title);
    }

    return projectSaved;
}

void MainWindow::updateAbsRateChart(std::shared_ptr<CalculatedData> DDResult)
{
    if (!DDResult)
        return;

    if (!proj)
        proj = std::make_shared<ProjectData>();

    const auto regionArray  = proj->regionArray;
    const auto regionNumber = proj->regionNumber;
    const auto group        = proj->energyGroup;

    long double maxAbpRateValue = 0.0;
    double totalRegionSize = 0.0;
    int nodex = 0;
    const auto absorptionRatePerNode = std::move(DDResult->absorptionRatePerNode);

    for (int rIndex = 0; rIndex < regionNumber; ++rIndex)
    {
        totalRegionSize += regionArray[rIndex].quote;
        nodex += proj->regionArray[rIndex].node;
    }

    ui->widgetNeutronAbsorpt->clearChart();

    for (int g = 0; g < group; ++g)
    {
        QList<QPointF> points;

        double positionX = 0.0;
        double stepSize = totalRegionSize / static_cast<double>(nodex);

        for (int nod = 0; nod < nodex; ++nod)
        {
            long double abpValue = absorptionRatePerNode[g][nod];
            maxAbpRateValue = std::max(maxAbpRateValue, abpValue);

            QPointF point(positionX, abpValue);
            points.append(point);

            positionX += stepSize;
        }

        ui->widgetNeutronAbsorpt->addSeries(points, g);
    }

    ui->widgetNeutronAbsorpt->setRange(0, 0, totalRegionSize, maxAbpRateValue);
    ui->widgetNeutronAbsorpt->setMaxPeriodicity(totalRegionSize);
    ui->widgetNeutronAbsorpt->setFilteredByGroup(group);
}

void MainWindow::updateAbsRateTable(std::shared_ptr<CalculatedData> DDResult)
{
    const auto regionNumber = proj->regionNumber;
    const auto energyGroup  = proj->energyGroup;

    QStringList regions;
    for (int rIndex = 1; rIndex <= regionNumber; ++rIndex)
    {
        regions << "Region " + QString::number(rIndex);
    }

    QStringList groups;
    for (int groupIndex = 1; groupIndex <= energyGroup; ++groupIndex)
    {
        groups << "Group " + QString::number(groupIndex);
    }

    ui->widgetNeutronAbsorpt->setTableDimension(energyGroup, regions.size());
    ui->widgetNeutronAbsorpt->setTableHeaders(regions, groups);
    ui->widgetNeutronAbsorpt->setTableItems(std::move(DDResult->absorptionRate));
}

void MainWindow::init()
{
    calculationThread = new QThread(this);
    worker = new Worker();
    worker->moveToThread(calculationThread);

    setConnections();

    qApp->setApplicationName("NeutronFluxCalculator");

    QApplication::setPalette(Interface::getDarkPalette());

    this->setWindowTitle(Interface::getWindowTitle());

    ui->tabWidget->setCurrentIndex(tabInputData);

    ui->widgetNeutronScalarFlux->setProjectionTitle("Scalar Flux of Neutral particles (DD method)");
    ui->widgetNeutronScalarFlux->setLabels("Position x (cm)", "Scalar Flux");

    ui->widgetNeutronAbsorpt->setProjectionTitle("Neutron Absorption Rate");
    ui->widgetNeutronAbsorpt->setLabels("Position x (cm)", "Rate");

    enableGenerateFilesMenu();

    ui->widgetRegion->setEnableGUI(false);

    //Open default project
    QTimer::singleShot(1000, this, [&]{    openProject();});
}

void MainWindow::showDefaultProjectWarning()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Default Project Notice");

    msgBox.setText("This project is a default example and cannot be modified. "
                   "If you want to create your own project, please save this example to a folder first.");

    msgBox.addButton(QMessageBox::Ok);
    msgBox.addButton(QMessageBox::Save);


    int ret = msgBox.exec();

    if (ret == QMessageBox::Save)
    {
        saveProjectFileDlg();
    }
}

void MainWindow::setConnections()
{
    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::changeFont);
    connect(ui->actionOpen_Project, &QAction::triggered, this, &MainWindow::openProjectFileDlg);
    connect(ui->actionPalette, &QAction::triggered, this, &MainWindow::changePalette);
    connect(ui->actionSave_Project, &QAction::triggered, this, &MainWindow::saveProjectFileDlg);
    connect(ui->actionScreenMode, &QAction::triggered, this, &MainWindow::changeViewMode);
    connect(ui->widgetRegion, &RegionInputData::onCalculateScalarNeutronFlux,
            this, &MainWindow::calculateNeutronFluxUsingDD);

    connect(ui->actionAbsorption_Cross_Section, &QAction::triggered, this, [this]()
            {
                showDataInFile(proj->absorptionCrossSectionFilePath);
            });

    connect(ui->actionScattering_Cross_Section, &QAction::triggered, this, [this]()
            {
                showDataInFile(proj->totalScatteringCrossSectionFilePath);
            });

    connect(ui->actionScalar_Flux, &QAction::triggered, this, [this]()
            {
                showDataInFile(proj->scalarFluxFile);
            });

    connect(ui->actionAbsorption_Rate, &QAction::triggered, this, [this]()
            {
                showDataInFile(proj->absorptionRateFile);
            });

    connect(ui->actionAbsorption_Rate_Per_Node, &QAction::triggered, this, [this]()
            {
                showDataInFile(proj->absorptionRatePerNodeFile);
            });

    connect(ui->actionAverage_Neutron_Flux_Per_Region, &QAction::triggered, this, [this]()
            {
                showDataInFile(proj->averageNeutronFluxPerRegionFile);
            });

    connect(ui->actionThe_app, &QAction::triggered, this, [this](){
        QMessageBox::information(this, "About", Interface::getAboutApp());
    });

    //Thread
    connect(worker, &Worker::outputData, this, &MainWindow::onOutputData);

    connect(worker, &Worker::finished, this, [this]()
            {
                this->statusBar()->showMessage("Finished");
                ui->widgetRegion->setPushButtonCalculateFluxEnable(true);
                QTimer::singleShot(2000, this, [&](){ this->statusBar()->showMessage(""); });
            });

    connect(worker, &Worker::errorOccurred, this, [this](auto errors)
            {
                QMessageBox::information(this, "Error", errors);
            });

    connect(this, &MainWindow::startProcess, worker, &Worker::process);
}

void MainWindow::startWork()
{
    if (!calculationThread->isRunning())
    {
        calculationThread->start();
    }
}

void MainWindow::stopWork()
{
    if (calculationThread->isRunning())
    {
        calculationThread->requestInterruption();
        calculationThread->quit();
        calculationThread->wait();
    }
}

void MainWindow::updateFluxChart(std::shared_ptr<CalculatedData> DDResult)
{
    if (!DDResult)
        return;

    if (!proj)
        proj = std::make_shared<ProjectData>();

    const auto regionArray  = proj->regionArray;
    const auto regionNumber = proj->regionNumber;
    const auto group        = proj->energyGroup;

    long double maxY = 0.0;
    double totalRegionSize = 0.0;
    int nodex = 0;
    const auto scalarFlux = std::move(DDResult->scalarFlux);

    for (int rIndex = 0; rIndex < regionNumber; ++rIndex)
    {
        totalRegionSize += regionArray[rIndex].quote;
        nodex += proj->regionArray[rIndex].node;
    }

    ui->widgetNeutronScalarFlux->clearChart();

    for (int g = 0; g < group; ++g)
    {
        QList<QPointF> points;

        double positionX = 0.0;
        double stepSize = totalRegionSize / static_cast<double>(nodex);

        for (int nod = 0; nod < nodex; ++nod)
        {
            long double fluxValue = scalarFlux[g][nod];
            maxY = std::max(maxY, fluxValue);

            QPointF point(positionX, fluxValue);
            points.append(point);

            positionX += stepSize;
        }

        ui->widgetNeutronScalarFlux->addSeries(points, g);
    }

    ui->widgetNeutronScalarFlux->setRange(0, 0, totalRegionSize, maxY);
    ui->widgetNeutronScalarFlux->setMaxPeriodicity(totalRegionSize);
    ui->widgetNeutronScalarFlux->setFilteredByGroup(group);

    ui->tabWidget->setCurrentIndex(TabResult);
}

void MainWindow::updateFluxTable(std::shared_ptr<CalculatedData> DDResult)
{
    const auto regionNumber = proj->regionNumber;
    const auto energyGroup = proj->energyGroup;

    QStringList regions;
    for (int rIndex = 1; rIndex <= regionNumber; ++rIndex)
    {
        regions << "Region " + QString::number(rIndex);
    }

    QStringList groups;
    for (int groupIndex = 1; groupIndex <= energyGroup; ++groupIndex)
    {
        groups << "Group " + QString::number(groupIndex);
    }

    ui->widgetNeutronScalarFlux->setTableDimension(energyGroup, regions.size());
    ui->widgetNeutronScalarFlux->setTableHeaders(regions, groups);
    ui->widgetNeutronScalarFlux->setTableItems(std::move(DDResult->averageNeutronFluxPerRegion));
}

void MainWindow::enableGenerateFilesMenu()
{
    ui->menuGenerated_Files->setEnabled(true);
}

