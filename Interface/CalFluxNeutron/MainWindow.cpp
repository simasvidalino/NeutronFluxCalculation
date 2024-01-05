#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include "InterFaceDefinitions.h"
#include "VariablesUsed.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QLineSeries>

#include <iostream>

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

        this->setPalette(Interface::getDarkPalette());
    }
    else
    {
        ui->actionPalette->setText(darkText);

        this->setPalette(Interface::getLightPalette());
    }
}

void MainWindow::init()
{
    setConnections();

    ui->widgetChart->setProjectionTitle("Scalar Flux of Neutral particles (DD method)");
    ui->widgetChart->setXLabel("Position x (cm)");
    ui->widgetChart->setYLabel("Scalar Flux");
}

void MainWindow::setConnections()
{
    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::changeFont);
    connect(ui->actionPalette, &QAction::triggered, this, &MainWindow::changePaletteToDarkStyle);
    connect(ui->actionScreenMode, &QAction::triggered, this, &MainWindow::changeViewMode);  // Coloca a janela em fullscreen
    connect(ui->widgetRegion, &RegionInputData::updateChartSignal, this, &MainWindow::updateChart);
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

    ui->tabWidget->setCurrentIndex(tabInputData);
}


