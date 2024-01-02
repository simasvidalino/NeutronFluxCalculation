#include "RegionInputData.h"
#include "ui_RegionInputData.h"

#include <QGraphicsRectItem>
#include <QDoubleSpinBox>
#include <QGraphicsProxyWidget>
#include <QLineEdit>

#include "DataMatrices.h" //construir dados de entrada
#include "DDNumericalMethold.h" //método Diamond Difference
#include "VariablesUsed.h"

#define sizeBar 100;

RegionInputData::RegionInputData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegionInputData),
    scene(std::make_unique<QGraphicsScene>(this))
{
    ui->setupUi(this);

    setConnections();

    init();
}

RegionInputData::~RegionInputData()
{
    delete ui;
}

std::vector<int> RegionInputData::calculateRegionHeights()
{
    std::vector<int> heights;
    auto extractValue = [](const regionData& myStruct) { return myStruct.quote; };

    if (regionQuant == 1)
    {
        heights.push_back(50);

        return heights;
    }

    // Calculate the sum of all quotas
    int sum = std::accumulate(std::begin(regionArray), std::begin(regionArray) + regionQuant, 0,
                              [&extractValue](int partialSum, const regionData& myStruct) {
        return partialSum + extractValue(myStruct);
    });

    int totalHeight = ui->graphicsView->height() -100;

    for (int iIndex = 0; iIndex < regionQuant; ++iIndex)
    {
        int height =  totalHeight*regionArray.at(iIndex).quote/sum;
        heights.push_back(height);
    }

    return heights;
}

void RegionInputData::clear()
{
    init();
}

void RegionInputData::calculateEscalarNeutronFlux()
{
    construir_dados("/home/andreiasimas/Documentos/TCCFluxEscalar/Calc_fluxo_de_particulas_neutras/CalNeutFlux/Arquivo_de_entrada.txt", valor);
    DD();
}

void RegionInputData::init()
{

    regionData dataInitial{50, 50};

    std::fill(regionArray.begin(), regionArray.end(), dataInitial);

    for (int iIndex = 0; iIndex < 10; ++iIndex)
    {
        auto quote = new QDoubleSpinBox;

        quoteSpinBoxes.at(iIndex) = quote;
    }

    ui->spinBoxRegionQtt->setValue(1);
}

void RegionInputData::setConnections()
{
    connect(ui->spinBoxRegionQtt, &QSpinBox::valueChanged, this, &RegionInputData::setGraphicScene);
    connect(ui->pushButtonClear, &QPushButton::clicked, this,  &RegionInputData::clear);
    connect(ui->pushButtonCalculateFlux, &QPushButton::clicked, this,
            &RegionInputData::calculateEscalarNeutronFlux);
}

void RegionInputData::setGraphicScene(int region)
{
    if (!scene)
        scene = std::make_unique<QGraphicsScene>();
    else
        scene->clear();

    regionQuant = region;

    std::vector<int> heights = calculateRegionHeights();

    int top = 0;
    int width  = 50;

    for (int iIndex = 0; iIndex < region; ++iIndex)
    {
        int height = heights.at(iIndex);

        setRegion(iIndex, 0, top, width, height);
        setQuota(iIndex,  0, top, width, height);

        top += height;
    }

    ui->graphicsView->setScene(scene.get());
}

void RegionInputData::setRegion(int regionNumber, int left, int top, int width, int height)
{
    QGraphicsRectItem *rectItem = new QGraphicsRectItem( left, top, width, height);

    scene->addItem(rectItem);
}

void RegionInputData::setQuota(int regionNumber, int left, int top, int width, int height)
{
    if (regionArray.size() <= regionNumber)
        return;

    int begin = left + width + 10;

    scene->addLine(begin, top, 1.5*begin, top);
    scene->addLine(begin, top + height, 1.5*begin, top + height);

    regionArray.at(regionNumber).region = regionNumber;

    setSpinBoxQuota(regionNumber, left, top, width, height);
}

void RegionInputData::setSpinBoxQuota(int regionNumber, int left, int top, int width, int height)
{
    QDoubleSpinBox *quote = new QDoubleSpinBox; //in cm
    QFont quoteFont("Arial", 9);

    quote->setDecimals(1);
    quote->setRange(2, 999); //@TBD
    QLineEdit *lineEdit = quote->findChild<QLineEdit*>(); //@TBDprotected member, the right way is create a child class
    lineEdit->setValidator(new QDoubleValidator(2, 999, 1, quote));
    quote->setFrame(false);
    quote->setSuffix("cm");
    quote->setFont(quoteFont);
    quote->setButtonSymbols(QAbstractSpinBox::NoButtons);
    quote->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    quote->setValue(regionArray.at(regionNumber).quote);
    quote->setObjectName(QString::number(regionNumber));
    quote->setStyleSheet("border: 0px solid black; border-bottom-width: 2px;");

    connect(quote, &QDoubleSpinBox::valueChanged, this,
            [quote, this](double value)
    {
        bool ok = false;
        int quoteIndex = quote->objectName().toInt(&ok);
        regionArray.at(quoteIndex).quote = value;

        emit quote->destroyed(); //TBD - failure when using the wheel event. Issuing this signal avoids the error

        setGraphicScene(regionQuant);

        if (!ok)
            qWarning()<<"Convertion QString to double error.";
    });

    QGraphicsProxyWidget *proxyWidget = new QGraphicsProxyWidget;
    proxyWidget->setWidget(quote);

    qreal rotationAngle = 90;
    proxyWidget->resize(height - 2, width - 2);
    proxyWidget->setRotation(rotationAngle);

    int begin = 2*(left + width + 10);
    proxyWidget->setPos(begin, top + 2);

    scene->addItem(proxyWidget);
}
