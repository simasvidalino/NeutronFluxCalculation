#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <iostream>

#include "NeutronFlowJsonIO.h"
#include "VariablesUsed.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QLineSeries>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(
                &ok, QFont("Helvetica [Cronyx]", 10), this);
    if (ok)
    {
        this->setFont(font);
    } else
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
        ui->actionPalette->setText("Color scheme to Default");
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
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(), filter);

    if (fileName.isEmpty())
        return;

    NeutronFlowJsonIO::getInstance()->loadProject(Interface::jsonFormat, fileName);
    auto proj = NeutronFlowJsonIO::getInstance()->getGeneralProjectData();

    if (!proj)
        proj = std::make_unique<ProjectData>();

    proj->regionArray =  std::move(NeutronFlowJsonIO::getInstance()->getRegionArray());

    ui->widgetChart->setPeriodicity(proj->periodicity);
    ui->widgetRegion->setGeneralProjectData(std::move(proj));
}

void MainWindow::saveProject()
{
    QString filter = "JSON Files (*.json);;Text Files (*.txt)";
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), filter);

    if (fileName.isEmpty())
        return;

    auto proj = ui->widgetRegion->getGeneralProjectData();

    if (!proj)
        proj = std::make_unique<ProjectData>();

    //Get periodicity
    proj->periodicity = ui->widgetChart->getPeriodicity();

    NeutronFlowJsonIO::getInstance()->setRegionArray(std::move(proj->regionArray));
    NeutronFlowJsonIO::getInstance()->setGeneralProjectData(std::move(proj));
    NeutronFlowJsonIO::getInstance()->saveProject(Interface::jsonFormat, fileName);
}

void MainWindow::updateAbsRateChart(std::shared_ptr<CalculatedData> DDResult)
{
    std::unique_lock<std::mutex> lock(mtx2);

    if (DDResult == nullptr)
        return;

    NeutronFlowJsonIO::getInstance()->loadProject(Interface::jsonFormat, fileName);
    auto proj = NeutronFlowJsonIO::getInstance()->getGeneralProjectData();
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
    ui->widgetChart->setFilterByGroup();
    ui->widgetChartAbsorptionRate->showPeriodicity();
    ui->widgetChartAbsorptionRate->setChart();
}


void MainWindow::init()
{
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

    //TBD qApp->setStyleSheet(tooltipStyle);
}

void MainWindow::setConnections()
{
    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::changeFont);
    connect(ui->actionOpen_Project, &QAction::triggered, this, &MainWindow::openProject);  // Coloca a janela em fullscreen
    connect(ui->actionPalette, &QAction::triggered, this, &MainWindow::changePaletteToDarkStyle);
    connect(ui->actionSave_Project, &QAction::triggered, this, &MainWindow::saveProject);  // Coloca a janela em fullscreen
    connect(ui->actionScreenMode, &QAction::triggered, this, &MainWindow::changeViewMode);  // Coloca a janela em fullscreen
    connect(ui->widgetRegion, &RegionInputData::updateChartSignal, this, &MainWindow::updateChart);
  //  connect(ui->widgetRegion, &RegionInputData::updateAbsChartSignal, this, &MainWindow::updateAbsRateChart);
    connect(ui->widgetChart, &ChartView::updatePeriodicity, this, [this](int value)
    {
        periodicityValue = value;
        updateChart();
    });

    connect(ui->actionThe_app, &QAction::triggered, this, [this](){
        QMessageBox::information(this, "About", Interface::getAboutApp());
    });


}

void MainWindow::updateChart()
{
    auto values = ui->widgetRegion->getDdValues();

    if (values == nullptr)
        return;

    int i   = 0;
    float t = 0;
    double maxY = 1; //@TBD
    QList<int> valueX;
    const int group = ui->widgetRegion->getNumberOfGroup();
    double totalRegionSize = 0.0;

    for (int rIndex = 0; rIndex < values->n_R; ++rIndex)
    {
        totalRegionSize += values->TAM[rIndex];
    }

    while(t <= totalRegionSize)
    {
        valueX.append(t);
        t = t + periodicityValue;
        i++;
    }

    ui->widgetChart->clearChart();

    long double maxFlux = 0;

    for(int g = 0; g < group; ++g)
    {
        QList<QPointF> points;
        int nod = 0;

        for(int n = 0; n < i; ++n)
        {
            long double fluxValues = values->FLUXO_ESCALAR[g][nod];

            maxFlux = std::max(maxFlux, fluxValues);

            QPointF point(valueX.at(n), fluxValues);
            points.append(point);

            qInfo()<<point<<nod;
            nod = nod + (values->NODOSX * periodicityValue)
                    /values->NODOSX;
        }

        ui->widgetChart->setInputData(points, group);
        ui->widgetChart->setTickNumber(i);
    }

    qInfo()<<"foi";

    ui->widgetChart->setXRange(0, totalRegionSize);
    ui->widgetChart->setYRange(0, maxFlux + 1);
    ui->widgetChart->setFilterByGroup();
    ui->widgetChart->showPeriodicity();

    std::cout<<"tick number "<<totalRegionSize<<" "<<group<<std::endl;
    ui->widgetChart->setChart();

    values.reset();

    ui->tabWidget->setCurrentIndex(TabResult);
}


