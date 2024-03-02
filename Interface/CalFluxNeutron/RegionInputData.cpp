#include "RegionInputData.h"
#include "CrossSectionFileDlg.h"
#include "ui_RegionInputData.h"

#include <QGraphicsRectItem>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QThread>

#include "DataMatrices.h" //construir dados de entrada
#include "DDNumericalMethold.h" //método Diamond Difference
#include "InterFaceDefinitions.h"
#include "VariablesUsed.h"
#include "MapRegion.h"
#include "TableInputDlg.h"

#define sizeBar 100;

RegionInputData::RegionInputData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegionInputData),
    scene(std::make_unique<QGraphicsScene>(this))
{
    ui->setupUi(this);

    setConnections();

    init();

    proj = std::make_unique<Interface::projetData>();
}

RegionInputData::~RegionInputData()
{
    delete ui;
}
void RegionInputData::setGeneralProjectData(std::unique_ptr<Interface::projetData> &&proj)
{
    this->proj = std::move(proj);

    loadGUI();
}

std::unique_ptr<Interface::projetData>&& RegionInputData::getGeneralProjectData()
{
    saveGUI();

    return std::move(proj);
}

std::shared_ptr<dados_entrada> RegionInputData::getDdValues() const
{
    return DDValues;
}

std::vector<int> RegionInputData::calculateRegionHeights()
{
    std::vector<int> heights;
    auto extractValue = [](const Interface::regionData& myStruct) { return myStruct.quote; };

    if (regionQuant == 1)
    {
        heights.push_back(50);

        return heights;
    }

    // Calculate the sum of all quotas
    int sum = std::accumulate(std::begin(regionArray), std::begin(regionArray) + regionQuant, 0,
                              [&extractValue](int partialSum, const Interface::regionData& myStruct)
    {
        return partialSum + extractValue(myStruct);
    });

    int totalHeight = ui->graphicsView->height() - 100;

    for (int iIndex = 0; iIndex < regionQuant; ++iIndex)
    {
        int height =  totalHeight*regionArray.at(iIndex).quote/sum;
        heights.push_back(height);
    }

    return heights;
}

void RegionInputData::clear()
{
    if (!scene)
        scene = std::make_unique<QGraphicsScene>();
    else
        scene->clear();

    init();
}

void RegionInputData::mapRegions()
{

}

void RegionInputData::onCreateCrossSectionFile()
{
    int group = ui->spinBoxGroup->value();

    if (group == 0)
    {
        QMessageBox::information(this, "Warning", "Energy Group is zero");
        return;
    }

    CrossSectionFileDlg dlg(this,
                            allZonasStr,
                            group,
                            ui->spinBoxLegendreOrder->value());

    dlg.loadCrossSectionFile(proj->scateringFilePath);

    if (!dlg.exec())
        return;

    scatteringPath = dlg.getPathCrossSection();
}

void RegionInputData::onNJOYClicked()
{

}

void RegionInputData::onOpenBCRightInputTable()
{
    TableInputDlg dlg(this);
    const int rowCount = ui->spinBoxGroup->value(); //energy group
    //tbd auto& bcRight = proj->bcRight;

    dlg.configTable(rowCount, QString("Right Boundary Conditions"), QString("Group"));

    if (bcRight.has_value())
        dlg.setColumnValues(0, bcRight.value());

    if (!dlg.exec())
        return;

    bcRight = dlg.getColumnValues(0);
}

void RegionInputData::onOpenBCLeftInputTable()
{
    TableInputDlg dlg(this);
    const int rowCount = ui->spinBoxGroup->value(); //energy group
    //tbd auto& bcLeft = proj->bcLeft;

    dlg.configTable(rowCount, QString("Left Boundary Conditions"), QString("Group"));

    if (bcLeft.has_value())
        dlg.setColumnValues(0, bcLeft.value());

    if (!dlg.exec())
        return;

    bcLeft = dlg.getColumnValues(0);
}

void RegionInputData::calculateEscalarNeutronFlux()
{
    //@todo change input and output of data
    // Calcular taxa de abs nas regiões
    // Escolher o grau de legendre.

    saveGUI();

    QThread *thread = new QThread();
    Worker *worker = new Worker();
    worker->moveToThread(thread);
    worker->setProjData(*proj);

    connect(worker, &Worker::finished, thread, &QThread::quit);
    connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::started, worker, &Worker::process);
    connect(worker, &Worker::outputData, this, [this](std::shared_ptr<CalculatedData> data)
            {
        DDOutputValues = data;
        emit updateChartSignal(); });

    //TBD delete
    connect(worker, &Worker::entrytData, this, [this](std::shared_ptr<dados_entrada> data)
            {
        DDValues = data;
        emit updateChartSignal(); });

    connect(worker, &Worker::absorptionRate, this, [this](std::vector<long double> absRate)
            {
        DDOutputValues->absorptionRate.swap(absRate);
        emit updateAbsChartSignal(); });

    thread->start();
}

void RegionInputData::onSelectionRegionChange()
{
    auto selectedItems = ui->graphicsView->scene()->selectedItems();
    auto groupNumber    = ui->spinBoxGroup->value();

    if (!selectedItems.empty())
    {
        auto rectItem = static_cast<QGraphicsRectItem*>(selectedItems.at(0));
        auto itemNameVariant = rectItem->data(0);
        bool ok = false;

        int number = itemNameVariant.toString().toInt(&ok);

        qInfo()<<"saiu"<<itemNameVariant<<ok;

        if (itemNameVariant.isValid() && ok)
        {
            //Create the dialog
            MapRegion dlg(this, regionQuant, groupNumber);

            auto regionPtr = std::make_unique<Interface::regionData>(regionArray.at(number));

            dlg.setRegionData(std::move(regionPtr));
            dlg.setAllZonasStr(allZonasStr);

            if (!dlg.exec())
                return;

            qInfo()<<"indice"<<number<<"array size"<<regionArray.size();

            if (number < regionArray.size())
            {
                auto &region = regionArray.at(number);
                auto regionDataPtr = dlg.getRegionData();

                if (regionDataPtr)
                {
                    //Feed region data
                    region = *std::move(regionDataPtr);
                    region.region = number; //TBD do we need a number ?
                    region.materialColor = zoneColors.at(region.zone);

                    //feed all zone data
                    allZonasStr = dlg.getAllZonasStr();

                    rectItem->setBrush(region.materialColor);

                    qInfo()<<"Zone selected"<<region.zone<<region.zoneStr;
                    rectItem->setSelected(false); //TBD the way you do that is strange
                }
            }
        }
        else
        {
            qWarning() << "No name associated with the item.";
        }
    }
}

void RegionInputData::init()
{
    ui->buttonGroupLeftBoundaryConditions->setId(ui->radioButtonBCLeftVacuum, 0);
    ui->buttonGroupLeftBoundaryConditions->setId(ui->radioButtonBCLeftPrescribed, 1);
    ui->buttonGroupLeftBoundaryConditions->setId(ui->radioButtonBCLeftReflexive, 2);

    ui->buttonGroupRightBoundaryConditions->setId(ui->radioButtonBCRightVacuum, 0);
    ui->buttonGroupRightBoundaryConditions->setId(ui->radioButtonBCRightPrescribed, 1);
    ui->buttonGroupRightBoundaryConditions->setId(ui->radioButtonBCRightReflexive, 2);

    ui->graphicsView->setPalette(Interface::getLightPalette()); //@TBD Even with the dark palette, the graphics must be clear

    Interface::regionData dataInitial{50, 50};

    std::fill(regionArray.begin(), regionArray.end(), dataInitial);

    for (int iIndex = 0; iIndex < 10; ++iIndex)
    {
        auto quote = new QDoubleSpinBox;

        quoteSpinBoxes.at(iIndex) = quote;
    }

    // scene->setSceneRect(ui->graphicsView->geometry());  // Substitua 'width' e 'height' pelos valores apropriados

    ui->spinBoxRegionQtt->setValue(0);
}

void RegionInputData::setConnections()
{
    connect(ui->pushButtonAddLeftPrecribedBCValues, &QPushButton::clicked, this, &RegionInputData::onOpenBCLeftInputTable);
    connect(ui->pushButtonAddRightPrecribedBCValues, &QPushButton::clicked, this, &RegionInputData::onOpenBCRightInputTable);

    connect(ui->pushButtonCreateCrossSection, &QPushButton::clicked, this, &RegionInputData::onCreateCrossSectionFile);

    connect(ui->pushButtonMapRegions, &QPushButton::clicked, this, &RegionInputData::mapRegions);
    connect(ui->spinBoxRegionQtt, &QSpinBox::valueChanged, this, &RegionInputData::setGraphicScene);
    connect(ui->pushButtonClear, &QPushButton::clicked, this,  &RegionInputData::clear);

    connect(ui->pushButtonCalculateFlux, &QPushButton::clicked, this,
            &RegionInputData::calculateEscalarNeutronFlux);

    connect(scene.get(), &QGraphicsScene::selectionChanged, this,
            &RegionInputData::onSelectionRegionChange);

    connect(ui->buttonGroupLeftBoundaryConditions, &QButtonGroup::buttonClicked,
            this, [this](auto button)
    {
        bool enable = false;

        if (button == ui->radioButtonBCLeftPrescribed)
            enable = true;
        else if (button == ui->radioButtonBCLeftReflexive)
            bcLeft.reset();
        else
            bcLeft->clear();

        ui->pushButtonAddLeftPrecribedBCValues->setEnabled(enable);
    });

    connect(ui->buttonGroupRightBoundaryConditions, &QButtonGroup::buttonClicked,
            this, [this](auto button)
    {
        bool enable = false;

        if (button == ui->radioButtonBCRightPrescribed)
            enable = true;
        else if (button == ui->radioButtonBCRightReflexive)
            bcRight.reset();
        else
            bcRight->clear();

        if(button == ui->radioButtonBCRightPrescribed)
            enable = true;
        ui->pushButtonAddRightPrecribedBCValues->setEnabled(enable);
    });
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
    auto materialColor = regionArray[regionNumber].materialColor;

    rectItem->setFlag(QGraphicsItem::ItemIsSelectable);

    rectItem->setData(0, QString::number(regionNumber));

    qInfo()<<"region "<<regionNumber<<materialColor<<materialColor.name();

    rectItem->setBrush(materialColor);

    scene->addItem(rectItem);
}

void RegionInputData::setQuota(int regionNumber, int left, int top, int width, int height)
{
    if (regionArray.size() <= regionNumber)
        return;

    int begin = left + width + 10;

    scene->addLine(begin, top, 1.5*begin, top);
    scene->addLine(begin, top + height, 1.5*begin, top + height);

    regionArray.at(regionNumber).region = regionNumber; //@Todo load has error

    setSpinBoxQuota(regionNumber, left, top, width, height);
}

void RegionInputData::setSpinBoxQuota(int regionNumber, int left, int top, int width, int height)
{
    QDoubleSpinBox* quote = nullptr;

    foreach(QGraphicsItem* item, scene->items())
    {
        QGraphicsProxyWidget* proxy = dynamic_cast<QGraphicsProxyWidget*>(item);
        if (proxy)
        {
            QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(proxy->widget());
            if (spinBox && spinBox->objectName() == QString::number(regionNumber))
            {
                quote = spinBox;
                break;
            }
        }
    }

    if (nullptr == quote)
        quote = new QDoubleSpinBox;

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
    quote->setStyleSheet("background-color: white; "
                         "border: 0px solid black; border-bottom-width: 2px;"
                         "color: #00008B;");

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

void RegionInputData::setZonesLegend(int region)
{
    if (!scene)
        scene = std::make_unique<QGraphicsScene>();
    else
        scene->clear();

    int legendTopMargin = 10;
    for (int i = 0; i < region; ++i) {
        QGraphicsRectItem *legendItem = new QGraphicsRectItem(0, 0, 30, 30);
        legendItem->setBrush(zoneColors.at(i));
        scene->addItem(legendItem);
    }

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

void RegionInputData::updateDDValues()
{
    DDValues.reset();

    BuildMatrices matrices;
    string filePath = "/home/andreiasimas/Documentos/NeutronFluxCalculation/Interface/build/DadosEntrada/Dados_1D_MultiGrupo_PM_Siewert1993_R1Z1G6L3upscattering.txt";

    //Save the value in the struct
    matrices.setAnisotropyOrder(ui->spinBoxAnisotropyOrder->value());
    matrices.setQuadratureOrder(ui->spinBoxQuadratureOrder->value());
    matrices.setStopOrder(ui->spinBoxStopOrder->value());
    matrices.setRegionsNumber(ui->spinBoxRegionQtt->value());
    matrices.setMaterialNumber(allZonasStr.size());
    matrices.setIterationNumber(ui->spinBoxMaxNumberIteration->value());

    matrices.run(BuildMatrices::DataOriginType::eUserInterfaceDataAndTextFile, filePath, *DDValues);
    auto projectData = std::make_unique<Interface::projetData>();

    //    projectData->precision = ui->spinBoxPrecision->value();
    //    projectData->energyGroup = 2;
    //    projectData->rightBoundaryConditionsType =
    //            static_cast<Interface::eBoundaryConditionsType>(ui->buttonGroupRightBoundaryConditions->checkedId());
    //    projectData->leftBoundaryConditionsType =
    //            static_cast<Interface::eBoundaryConditionsType>(ui->buttonGroupLeftBoundaryConditions->checkedId());
    //    projectData->maximumIterationsNumber = ui->spinBoxMaxNumberInteration->value();

    DDValues = std::make_shared<dados_entrada>();

    //    qInfo()<<projectData->precision
    //          <<projectData->energyGroup
    //         <<projectData->leftBoundaryConditionsType
    //        <<projectData->maximumIterationsNumber;

    //    DDValues->G   = projectData->energyGroup;
    //    DDValues->L   = projectData->legendreOrder;
    //    DDValues->n_R = projectData->regionNumber;
    //    DDValues->n_Z = projectData->zoneNumber;

    //    //Save json
    //    NeutronFlowJsonIO::getInstance()->setGeneralProjectData(std::move(projectData));
}

void RegionInputData::loadGUI()
{
    if (!proj)
        proj = std::make_unique<Interface::projetData>();

    regionArray = std::move(proj->regionArray);

    ui->spinBoxRegionQtt->setValue(proj->regionNumber);
    ui->spinBoxMaxNumberIteration->setValue(proj->maximumIterationsNumber);
    ui->spinBoxGroup->setValue(proj->energyGroup);
    ui->spinBoxQuadratureOrder->setValue(proj->quadratureOrder);

    scatteringPath = QString::fromStdString(proj->scateringFilePath);

    QAbstractButton *bcLeftButton = ui->buttonGroupLeftBoundaryConditions->button(
                proj->leftBoundaryConditionsType);

    if(bcLeftButton)
        bcLeftButton->click();

    bcLeft.reset();
    if (proj->leftBoundaryConditionsType == Interface::ePrescribed)
        bcLeft = proj->bcLeft.value();

    QAbstractButton *bcRightButton = ui->buttonGroupRightBoundaryConditions->button(
                proj->rightBoundaryConditionsType);

    if(bcRightButton)
        bcRightButton->click();

    bcRight.reset();
    if (proj->rightBoundaryConditionsType == Interface::ePrescribed)
        bcRight = proj->bcRight.value();

    ui->spinBoxStopOrder->setValue(proj->stopOrder);

    allZonasStr.clear();
    for (int iIndex = 0; iIndex < proj->regionNumber; ++iIndex)
    {
        if (!regionArray.empty()
                || regionArray.size() > iIndex)
        {
            auto zoneStr = regionArray[iIndex].zoneStr;

            if (allZonasStr.contains(zoneStr) )
                    continue;

            allZonasStr.append(zoneStr);
        }
    }

    ui->spinBoxLegendreOrder->setValue(proj->legendreOrder);
}

void RegionInputData::saveGUI()
{
    if (!proj)
        proj = std::make_unique<Interface::projetData>();

    proj->regionArray             = regionArray;
    proj->regionNumber            = ui->spinBoxRegionQtt->value();
    proj->maximumIterationsNumber = ui->spinBoxMaxNumberIteration->value();
    proj->energyGroup             = ui->spinBoxGroup->value();

    int leftBC  =  ui->buttonGroupLeftBoundaryConditions->checkedId();
    int rightBC =  ui->buttonGroupRightBoundaryConditions->checkedId();

    if (leftBC == Interface::ePrescribed)
        proj->bcLeft = bcLeft;

    if (rightBC == Interface::ePrescribed)
        proj->bcRight = bcRight;

    proj->leftBoundaryConditionsType  = Interface::eBoundaryConditionsType(leftBC);
    proj->rightBoundaryConditionsType = Interface::eBoundaryConditionsType(rightBC);
    proj->scateringFilePath           = scatteringPath.toStdString();
    proj->stopOrder                   = ui->spinBoxStopOrder->value();
    proj->zoneNumber                  = allZonasStr.size();
    proj->legendreOrder               = ui->spinBoxLegendreOrder->value();
    proj->quadratureOrder             = ui->spinBoxQuadratureOrder->value();
}

int RegionInputData::getRegionQuant() const
{
    return regionQuant;
}
