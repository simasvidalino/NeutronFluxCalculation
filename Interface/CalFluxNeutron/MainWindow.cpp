#include "MainWindow.h"
#include "./ui_MainWindow.h"
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
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));           // Dark background color
        darkPalette.setColor(QPalette::WindowText, QColor(212, 212, 212));    // Light text color
        darkPalette.setColor(QPalette::Highlight, QColor(38, 79, 120));       // Highlight color (when pressed)
        darkPalette.setColor(QPalette::HighlightedText, QColor(212, 212, 212)); // Text color when highlighted
        darkPalette.setColor(QPalette::Base, QColor(51, 51, 51));             // Dark base color
        darkPalette.setColor(QPalette::AlternateBase, QColor(46, 46, 46));    // Alternative base color
        darkPalette.setColor(QPalette::Text, QColor(212, 212, 212));          // Default text color
        darkPalette.setColor(QPalette::Button, QColor(51, 51, 51));          // Button color
        darkPalette.setColor(QPalette::ButtonText, QColor(212, 212, 212));    // Text color for buttons
        darkPalette.setColor(QPalette::Link, QColor(85, 167, 255));

        ui->actionPalette->setText("Color scheme to Default");

        QApplication::setPalette(darkPalette);
    }
    else
    {
        QPalette lightPalette;
        lightPalette.setColor(QPalette::Window, QColor(255, 255, 255));      // Light background color
        lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));         // Dark text color
        lightPalette.setColor(QPalette::Highlight, QColor(173, 216, 230));    // Highlight color (when pressed)
        lightPalette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));    // Text color when highlighted
        lightPalette.setColor(QPalette::Base, QColor(245, 245, 245));         // Light gray base color
        lightPalette.setColor(QPalette::AlternateBase, QColor(235, 235, 235)); // Alternate base color
        lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));               // Default text color
        lightPalette.setColor(QPalette::Button, QColor(200, 200, 200));       // Button color
        lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));        // Button text color
        lightPalette.setColor(QPalette::Link, QColor(0, 122, 255));           // Apple Blue link color

        ui->actionPalette->setText(darkText);

        QApplication::setPalette(lightPalette);
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

    auto valor = ui->widgetRegion->getDdValues();


    double maxY = 1; //@TBD

    QList<int> valueX;

    while(t <= valor->TAM_TOTAL)
    {
        valueX.append(t);
        t = t + valor->periodicidade;
        i++;
    }


    for(int g = 0; g < valor->G; ++g)
    {
        QList<QPointF> points;

        int nod = 0;
        for(int n = 0; n < i; n++)
        {
            float fluxValor = valor->FLUXO_ESCALAR[g][nod];

            QPointF point(valueX.at(n), fluxValor);
            points.append(point);

            std::cout<<"( "<<nod<<" - "<<valor->FLUXO_ESCALAR[g][nod]<<" )  "<<point.x()<<std::endl;
            nod = nod + (valor->NODOSX*valor->periodicidade)/valor->TAM_TOTAL;
        }

        ui->widgetChart->setInputData(points, g);
        ui->widgetChart->setTickNumber(i);
    }



    std::cout<<"tick number "<<valor->TAM_TOTAL<<" "<<valor->G<<std::endl;
    ui->widgetChart->setChart();
}


