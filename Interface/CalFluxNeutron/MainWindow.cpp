#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <mutex>

#include "NeutronFlowJsonIO.h"
#include "VariablesUsed.h"
#include "qtimer.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QLineSeries>
#include <QMessageBox>
#include <QThread>
std::mutex mtx1;
std::mutex mtx2;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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

    NeutronFlowJsonIO::getInstance()->loadProject(Interface::jsonFormat, fileName);
    proj = NeutronFlowJsonIO::getInstance()->getGeneralProjectData();

    if (!proj)
        proj = std::make_unique<ProjectData>();

    worker->setProjData(*proj);

    this->statusBar()->showMessage("Calculating...");

    ui->widgetRegion->setPushButtonCalculateFluxEnable(false);

    //    QThread *thread = new QThread(this);
    //    Worker *worker = new Worker();
    //    worker->moveToThread(thread);
    //    worker->setProjData(*proj);

    //    connect(worker, &Worker::finished, thread, &QThread::quit);
    //    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    //    connect(worker, &Worker::finished, this, [this]()
    //    {
    //        ui->widgetRegion->setPushButtonCalculateFluxEnable(true);
    //    });

    //    connect(worker, &Worker::finished, worker, &Worker::deleteLater);

    //    connect(thread, &QThread::started, worker, &Worker::process);

    //    connect(worker, &Worker::outputData, this, [this](const auto data)
    //    {
    //        updateFluxChart(data);
    //        updateAbsRateChart(data);
    //    });

    //    thread->start();

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

void MainWindow::changePaletteToDarkStyle()
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

void MainWindow::openProject()
{
    QString filter = "JSON Files (*.json);;Text Files (*.txt)";
    fileName = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(), filter);

    if (fileName.isEmpty())
        return;

    QString title = QString(Interface::getWindowTitle()) + QString(": ") + fileName.split("/").back();
    this->setWindowTitle(title);

    this->statusBar()->showMessage("Waite...");
    ui->widgetRegion->setPushButtonCalculateFluxEnable(false);

    NeutronFlowJsonIO::getInstance()->loadProject(Interface::jsonFormat, fileName);
    auto proj = NeutronFlowJsonIO::getInstance()->getGeneralProjectData();

    if (!proj)
        proj = std::make_unique<ProjectData>();

    proj->regionArray =  std::move(NeutronFlowJsonIO::getInstance()->getRegionArray());

    ui->widgetChart->setPeriodicity(proj->periodicity);
    ui->widgetRegion->setGeneralProjectData(std::move(proj));

    QTimer::singleShot(2000, this, [&](){
        this->statusBar()->showMessage("Ready");
        ui->widgetRegion->setPushButtonCalculateFluxEnable(true);
    });
}

void MainWindow::saveProjectDlg()
{
    QString filter = "JSON Files (*.json);;Text Files (*.txt)";
    fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), filter);

    saveProject();
}

bool MainWindow::saveProject()
{
    bool projectSaved = false;

    if (fileName.isEmpty())
    {
        QMessageBox::information(this, "Project File Not Found", "Please save your project before proceeding.");
    }
    else
    {
        projectSaved = true;
        auto proj = ui->widgetRegion->getGeneralProjectData();

        //Get periodicity
        proj->periodicity = ui->widgetChart->getPeriodicity();

        NeutronFlowJsonIO::getInstance()->setRegionArray(std::move(proj->regionArray));
        NeutronFlowJsonIO::getInstance()->setGeneralProjectData(std::move(proj));
        NeutronFlowJsonIO::getInstance()->saveProject(Interface::jsonFormat, fileName);

        QString title = QString(Interface::getWindowTitle()) + QString(": ") + fileName.split("/").back();
        this->setWindowTitle(title);
    }

    return projectSaved;
}

void MainWindow::updateAbsRateChart(std::shared_ptr<CalculatedData> DDResult)
{
    std::unique_lock<std::mutex> lock(mtx2);

    if (DDResult == nullptr)
        return;

    const auto& regionArray = proj->regionArray;
    auto absorptionRate = DDResult->absorptionRate;
    int group = proj->energyGroup;

    long double maxFlux = 0.0;

    ui->widgetChartAbsorptionRate->clearChart();
    const int regionQtt = proj->regionNumber;
    double startPosition = 0.0;

    for (int g = 0; g < group; ++g)
    {
        QList<QPointF> points;
        startPosition = 0.0;

        for (int rIndex = 0; rIndex < regionQtt; ++rIndex)
        {
            auto absRateValue = absorptionRate[rIndex][g];
            maxFlux = std::max(maxFlux, absRateValue);

            points.append(QPointF(startPosition, absRateValue));

            startPosition += regionArray[rIndex].quote;

            points.append(QPointF(startPosition, absRateValue));
        }

        ui->widgetChartAbsorptionRate->setInputData(points, g);
    }

    ui->widgetChartAbsorptionRate->setXRange(0, startPosition);
    ui->widgetChartAbsorptionRate->setYRange(0, maxFlux);
    ui->widgetChartAbsorptionRate->showPeriodicity();
    ui->widgetChartAbsorptionRate->setChart();

    //setFilterByGroup(group);

    this->statusBar()->showMessage("Finished");

    QTimer::singleShot(2000, this, [&](){ this->statusBar()->showMessage(""); });
}

void MainWindow::init()
{
    qApp->setApplicationName("NeutronFluxCalculator");

    QApplication::setPalette(Interface::getDarkPalette());

    this->setWindowTitle(Interface::getWindowTitle());

    ui->tabInputData->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

    ui->tabWidget->setCurrentIndex(tabInputData);

    calculationThread = new QThread(this);
    worker = new Worker();
    worker->moveToThread(calculationThread);

    setConnections();

    ui->widgetChart->setProjectionTitle("Scalar Flux of Neutral particles (DD method)");
    ui->widgetChart->setXLabel("Position x (cm)");
    ui->widgetChart->setYLabel("Scalar Flux");

    ui->widgetChartAbsorptionRate->setProjectionTitle("Neutron Absorption Rate");
    ui->widgetChartAbsorptionRate->setXLabel("Position x (cm)");
    ui->widgetChartAbsorptionRate->setYLabel("Rate");

    QString tooltipStyle = "QToolTip {"
                           "  background-color: #F0F0F0;"
                           "  border: 1px solid #808080;"
                           "  padding: 2px;"
                           "};";


    //qApp->setStyleSheet(tooltipStyle);
}

void MainWindow::setConnections()
{
    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::changeFont);
    connect(ui->actionOpen_Project, &QAction::triggered, this, &MainWindow::openProject);
    connect(ui->actionPalette, &QAction::triggered, this, &MainWindow::changePaletteToDarkStyle);
    connect(ui->actionSave_Project, &QAction::triggered, this, &MainWindow::saveProjectDlg);
    connect(ui->actionScreenMode, &QAction::triggered, this, &MainWindow::changeViewMode);
    connect(ui->widgetRegion, &RegionInputData::onCalculateScalarNeutronFlux,
            this, &MainWindow::calculateNeutronFluxUsingDD);

    connect(ui->widgetChart, &ChartView::updatePeriodicity, this, [this](auto value)
    {
        periodicityValue = value;
        updateChartStep();
    });

    connect(ui->actionThe_app, &QAction::triggered, this, [this](){
        QMessageBox::information(this, "About", Interface::getAboutApp());
    });


    //Thread
    connect(worker, &Worker::outputData, this, [this](const auto& data)
    {
        updateFluxChart(data);
        updateAbsRateChart(data);
    });

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

    connect(ui->comboBoxFilter, &QComboBox::activated, this, [this](int index)
    {
        ui->widgetChart->filterChange(index);
    });
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

void MainWindow::setFilterByGroup(int group)
{
    QStringList options;
    options << "All";

    qInfo()<<"setFilterByGroup"<<group;

    for (int ig = 0; ig < group; ++ig)
    {
        options << "Group " + QString::number(ig + 1);
    }

    //if we have only one group we don't need a filter
    if (group == 1)
        ui->comboBoxFilter->hide();
    else
        ui->comboBoxFilter->show();

    ui->comboBoxFilter->setFixedWidth(300);

    ui->comboBoxFilter->clear();
    ui->comboBoxFilter->addItems(options);
}

void MainWindow::updateFluxChart(std::shared_ptr<CalculatedData> DDResult)
{
    std::unique_lock<std::mutex> lock(mtx1);

    if (!DDResult)
        return;

    if (!proj)
        proj = std::make_unique<ProjectData>();

    const auto regionArray  = proj->regionArray;
    const auto regionNumber = proj->regionNumber;
    const auto group        = proj->energyGroup;

    double maxY = 1; //@TBD
    double totalRegionSize = 0.0;
    int nodex = 0;
    const auto scalarFlux = std::move(DDResult->scalarFlux);

    for (int rIndex = 0; rIndex < regionNumber; ++rIndex)
    {
        totalRegionSize += regionArray[rIndex].quote;
        nodex += proj->regionArray[rIndex].node;
    }

    ui->widgetChart->clearChart();

    long double maxFlux = 0;
    for (int g = 0; g < group; ++g)
    {
        QList<QPointF> points;

        double positionX = 0.0;
        double stepSize = totalRegionSize / static_cast<double>(nodex); // Calcula o tamanho de cada passo baseado no total de nodos

        for (int nod = 0; nod < nodex; ++nod)
        {
            long double fluxValue = scalarFlux[g][nod];
            maxFlux = std::max(maxFlux, fluxValue);

            QPointF point(positionX, fluxValue);
            points.append(point);

            positionX += stepSize;
        }

        ui->widgetChart->setInputData(points, g);
    }

    ui->widgetChart->setXRange(0, totalRegionSize);
    ui->widgetChart->setYRange(0, maxFlux + 1);
    ui->widgetChart->showPeriodicity();
    ui->widgetChart->setChart();

    setFilterByGroup(group);

    ui->tabWidget->setCurrentIndex(TabResult);
}

void MainWindow::updateChartStep()
{
    NeutronFlowJsonIO::getInstance()->loadProject(Interface::jsonFormat, fileName);
    auto proj = NeutronFlowJsonIO::getInstance()->getGeneralProjectData();

    if (!proj)
        proj = std::make_unique<ProjectData>();

    double totalRegionSize = 0.0;

    for (int rIndex = 0; rIndex < proj->regionNumber; ++rIndex)
    {
        totalRegionSize += proj->regionArray[rIndex].quote;
    }

    int tickCount = totalRegionSize/periodicityValue;

    ui->widgetChart->setTickNumber(tickCount + 1);
    ui->widgetChart->setChart();
}
