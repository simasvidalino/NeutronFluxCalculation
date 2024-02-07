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


    //TBD test delete
    int amarelo = 16776960;
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
        proj = std::make_unique<Interface::projetData>();

    proj->regionArray =  std::move(NeutronFlowJsonIO::getInstance()->getRegionArray());
    ui->widgetRegion->setGeneralProjectData(std::move(proj));
}

void MainWindow::saveProject()
{
    QString filter = "Text Files (*.txt);;JSON Files (*.json)";
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::homePath(), filter);

    if (fileName.isEmpty())
        return;

    auto proj = ui->widgetRegion->getGeneralProjectData();

    if (!proj)
        proj = std::make_unique<Interface::projetData>();

    NeutronFlowJsonIO::getInstance()->setRegionArray(std::move(proj->regionArray));
    NeutronFlowJsonIO::getInstance()->setGeneralProjectData(std::move(proj));
    NeutronFlowJsonIO::getInstance()->saveProject(Interface::jsonFormat, fileName);
}

void MainWindow::init()
{
    setConnections();

    ui->widgetChart->setProjectionTitle("Scalar Flux of Neutral particles (DD method)");
    ui->widgetChart->setXLabel("Position x (cm)");
    ui->widgetChart->setYLabel("Scalar Flux");

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

    //connect(this, &MainWindow::onProject, ui->widgetRegion, &RegionInputData::onProjectSave);


    connect(ui->actionThe_app, &QAction::triggered, this, [this](){
        QMessageBox::information(this, "About", Interface::getAboutApp());
    });


}

void MainWindow::updateChart()
{
    int i = 0;
    float t = 0;

    auto values = ui->widgetRegion->getDdValues();

    double maxY = 1; //@TBD

    QList<int> valueX;
    QList<int> regionSize;

    for (int iIndex = 0; iIndex < values->n_R; ++iIndex)
        regionSize.append(values->TAM[iIndex]);

    while(t <= values->TAM_TOTAL)
    {
        valueX.append(t);
        qInfo()<<t;
        t = t + values->periodicidade;
        i++;
    }

    ui->widgetChart->clearChart();

    static int in = 0;

    for(int g = 0; g < values->G; ++g)
    {
        QList<QPointF> points;

        int nod = 0;
        for(int n = 0; n < i; n++)
        {
            float fluxvalues = values->FLUXO_ESCALAR[g][nod];

            if (in == 1)
                fluxvalues = 0.5;


            QPointF point(valueX.at(n), fluxvalues);
            points.append(point);

            std::cout<<"( "<<nod<<" - "<<values->FLUXO_ESCALAR[g][nod]<<" )  "<<point.x()<<std::endl;
            nod = nod + (values->NODOSX*values->periodicidade)/values->TAM_TOTAL;
        }

        ui->widgetChart->setInputData(points, regionSize, g);
        ui->widgetChart->setTickNumber(i);
    }

    ++in;


    std::cout<<"tick number "<<values->TAM_TOTAL<<" "<<values->G<<std::endl;
    ui->widgetChart->setChart();

    values.reset();

    ui->tabWidget->setCurrentIndex(TabResult);
}


