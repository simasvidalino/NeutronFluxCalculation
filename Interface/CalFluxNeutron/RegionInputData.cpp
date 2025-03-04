#include "RegionInputData.h"
#include "FileViewerDlg.h"
#include "ui_RegionInputData.h"

#include <QGraphicsRectItem>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QThread>

#include "InterFaceDefinitions.h"
#include "VariablesUsed.h"
#include "MapRegion.h"
#include "NJOYInputDlg.h"
#include "ProjectStructs.h"
#include "TableInputDlg.h"

#define sizeBar 100;

RegionInputData::RegionInputData(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::RegionInputData),
      scene(std::make_unique<QGraphicsScene>(this))
{
    ui->setupUi(this);

    //TBD
    QTimer::singleShot(1000, this, [&](){ init(); });

    //Future implementation
    ui->pushButtonNJOY->hide();
}

RegionInputData::~RegionInputData()
{
    delete ui;
}

void RegionInputData::setEnableGUI(bool enable)
{
    ui->groupBoxGeneralParameters->setEnabled(enable);
    ui->groupBoxBoundaryConditions->setEnabled(enable);
    ui->groupBoxIterativeProcess->setEnabled(enable);
    ui->groupBoxCrosSection->setEnabled(enable);
    ui->graphicsView->setEnabled(enable);
    ui->spinBoxRegionQtt->setEnabled(enable);
    ui->pushButtonClear->setEnabled(enable);
}

void RegionInputData::setGeneralProjectData(std::shared_ptr<ProjectData> proj)
{
    this->proj = proj;

    loadGUI();
}

std::shared_ptr<ProjectData> RegionInputData::getGeneralProjectData()
{
    saveGUI();

    return proj;
}

std::shared_ptr<dados_entrada> RegionInputData::getDdValues() const
{
    return DDValues;
}

const std::vector<std::vector<long double>> &RegionInputData::getScalarFlux() const
{
    return scalarFlux;
}

std::vector<int> RegionInputData::calculateRegionHeights()
{
    std::vector<int> heights;
    auto extractValue = [](const RegionData &myStruct)
    { return myStruct.quote; };

    if (regionQuant == 1)
    {
        heights.push_back(50);

        return heights;
    }

    // Calculate the sum of all quotas
    int sum = std::accumulate(std::begin(regionArray), std::begin(regionArray) + regionQuant, 0,
                              [&extractValue](int partialSum, const RegionData &myStruct)
    {
        return partialSum + extractValue(myStruct);
    });

    int totalHeight = ui->graphicsView->height() - 100;

    // Sometimes the height is wrong because Qt events was not released.
    if (totalHeight < 0)
        qWarning() << "ui->graphicsView->height() is negative";

    for (int iIndex = 0; iIndex < regionQuant; ++iIndex)
    {
        int height = totalHeight * regionArray.at(iIndex).quote / sum;
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

void RegionInputData::onCreateCrossSectionFile()
{
    int group = ui->spinBoxGroup->value();

    if (group == 0)
    {
        QMessageBox::information(this, "Warning", "Energy Group is zero");
        return;
    }

    FileViewerDlg dlg(this, allZonasStr.size(),
                            group,
                            ui->spinBoxLegendreOrder->value());

    auto pathStr = scatteringPath.toStdString();

    dlg.loadCrossSectionFile(pathStr);

    if (!dlg.exec())
        return;

    scatteringPath = dlg.getPathCrossSection();
    emit onCalculateCrossSectionMatrices();
}

void RegionInputData::onNJOYClicked()
{
    NJOYInputDlg dlg(this);

    if (!dlg.exec())
        return;
}

void RegionInputData::onOpenBCRightInputTable()
{
    TableInputDlg dlg(this);
    const int rowCount = ui->spinBoxGroup->value(); // energy group

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
    const int rowCount = ui->spinBoxGroup->value(); // energy group

    dlg.configTable(rowCount, QString("Left Boundary Conditions"), QString("Group"));

    if (bcLeft.has_value())
        dlg.setColumnValues(0, bcLeft.value());

    if (!dlg.exec())
        return;

    bcLeft = dlg.getColumnValues(0);
}

void RegionInputData::onSelectionRegionChange()
{
    auto selectedItems = ui->graphicsView->scene()->selectedItems();
    auto groupNumber = ui->spinBoxGroup->value();

    if (!selectedItems.empty())
    {
        auto rectItem = static_cast<QGraphicsRectItem *>(selectedItems.at(0));
        auto itemNameVariant = rectItem->data(0);
        bool ok = false;

        int number = itemNameVariant.toString().toInt(&ok);

        if (itemNameVariant.isValid() && ok)
        {
            // Create the dialog
            MapRegion dlg(this, regionQuant, groupNumber);

            auto regionPtr = std::make_unique<RegionData>(regionArray.at(number));

            dlg.loadRegionData(std::move(regionPtr));
            dlg.loadAllZonasStr(allZonasStr);

            if (!dlg.exec())
                return;

            qInfo() << "indice" << number << "array size" << regionArray.size();

            if (number < regionArray.size())
            {
                auto &region = regionArray.at(number);
                auto regionDataPtr = dlg.getRegionData();

                if (regionDataPtr)
                {
                    // Feed region data
                    region = *std::move(regionDataPtr);
                    region.region = number;                                                   // TBD is the way to get the Color ok?
                    region.materialColor = zoneColors.at(region.zone - 1).rgb() & 0x00FFFFFF; // Remove alpha chanel

                    // feed all zone data
                    allZonasStr = dlg.getAllZonasStr();

                    if (rectItem != nullptr)
                    {
                        rectItem->setBrush(QColor(region.materialColor));
                        rectItem->setSelected(false); // TBD the way you do that is strange
                    }

                    QCoreApplication::processEvents();

                    //TBD we should update the scene, but using only setGraphicScene function causes crahses
                    //Verify why
                    QTimer::singleShot(50, this, [&](){ setGraphicScene(regionQuant); });
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
    ui->graphicsView->rotate(-90);

    ui->buttonGroupLeftBoundaryConditions->setId(ui->radioButtonBCLeftVacuum, 0);
    ui->buttonGroupLeftBoundaryConditions->setId(ui->radioButtonBCLeftPrescribed, 1);
    ui->buttonGroupLeftBoundaryConditions->setId(ui->radioButtonBCLeftReflexive, 2);

    ui->buttonGroupRightBoundaryConditions->setId(ui->radioButtonBCRightVacuum, 0);
    ui->buttonGroupRightBoundaryConditions->setId(ui->radioButtonBCRightPrescribed, 1);
    ui->buttonGroupRightBoundaryConditions->setId(ui->radioButtonBCRightReflexive, 2);

    ui->graphicsView->setPalette(Interface::getLightPalette()); //@TBD Even with the dark palette, the graphics must be clear

    RegionData dataInitial{50, 50};

    std::fill(regionArray.begin(), regionArray.end(), dataInitial);

    setConnections();

    ui->pushButtonCreateCrossSection->setToolTip(Interface::getCrossSessionDataToolTip());


    //Accepts only even quadrature values.
    QLineEdit *lineEdit = ui->spinBoxQuadratureOrder->findChild<QLineEdit*>();

    if (lineEdit)
    {
        lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9]*[02468]$"), ui->spinBoxQuadratureOrder));

        connect(ui->spinBoxQuadratureOrder, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, lineEdit](int value) {
            if (value % 2 != 0)
            {
                ui->spinBoxQuadratureOrder->setValue(value + 1);
            }
        });
    }
}

void RegionInputData::setConnections()
{
    connect(ui->pushButtonAddLeftPrecribedBCValues, &QPushButton::clicked, this, &RegionInputData::onOpenBCLeftInputTable);
    connect(ui->pushButtonAddRightPrecribedBCValues, &QPushButton::clicked, this, &RegionInputData::onOpenBCRightInputTable);

    connect(ui->pushButtonCreateCrossSection, &QPushButton::clicked, this, &RegionInputData::onCreateCrossSectionFile);

    connect(ui->spinBoxRegionQtt, &QSpinBox::valueChanged, this, &RegionInputData::setGraphicScene);
    connect(ui->pushButtonClear, &QPushButton::clicked, this, &RegionInputData::clear);

    connect(ui->pushButtonNJOY, &QPushButton::clicked, this, &RegionInputData::onNJOYClicked);

    //all operations are done in the MainWindow so as not to overload this window.
    connect(ui->pushButtonCalculateFlux, &QPushButton::clicked, this, [this](){
        emit onCalculateCrossSectionMatrices();
        emit onCalculateScalarNeutronFlux();
    });

    connect(scene.get(), &QGraphicsScene::selectionChanged, this,
            &RegionInputData::onSelectionRegionChange);

    connect(ui->buttonGroupLeftBoundaryConditions, &QButtonGroup::buttonClicked,
            this, [this](auto button)
    {
        bool enable = false;

        if (button == ui->radioButtonBCLeftPrescribed)
            enable = true;
        else if (button == ui->radioButtonBCLeftReflexive
                 && bcLeft.has_value())
            bcLeft.reset();
        else if (bcLeft.has_value())
            bcLeft->clear();

        ui->pushButtonAddLeftPrecribedBCValues->setEnabled(enable); });

    connect(ui->buttonGroupRightBoundaryConditions, &QButtonGroup::buttonClicked,
            this, [this](auto button)
    {
        bool enable = false;

        if (button == ui->radioButtonBCRightPrescribed)
            enable = true;
        else if (button == ui->radioButtonBCRightReflexive
                 && bcRight.has_value())
            bcRight.reset();
        else if (bcRight.has_value())
            bcRight->clear();

        if(button == ui->radioButtonBCRightPrescribed)
            enable = true;
        ui->pushButtonAddRightPrecribedBCValues->setEnabled(enable); });
}

void RegionInputData::setGraphicScene(int region)
{
    if (!scene)
        scene = std::make_unique<QGraphicsScene>();
    else
        scene->clear();

    regionQuant = region;
    int top = 0;
    int width = 50;

    std::vector<int> heights = calculateRegionHeights();

    for (int iIndex = 0; iIndex < region; ++iIndex)
    {
        int height = heights.at(iIndex);

        setRegionGraphicsRectItem(iIndex, 0, top, width, height);
        setQuotaLinesGraphicsItem(iIndex, 0, top, width, height);
        setSpinBoxQuota(iIndex, 0, top, width, height);
        top += height;
    }

    ui->graphicsView->setScene(scene.get());
}

void RegionInputData::setRegionGraphicsRectItem(int regionNumber, int left, int top, int width, int height)
{
    QGraphicsRectItem *rectItem = new QGraphicsRectItem(left, top, width, height);
    auto materialColor = regionArray[regionNumber].materialColor;

    rectItem->setFlag(QGraphicsItem::ItemIsSelectable);

    rectItem->setData(0, QString::number(regionNumber));

    rectItem->setBrush(QColor(materialColor));

    scene->addItem(rectItem);
}

void RegionInputData::setQuotaLinesGraphicsItem(int regionNumber, int left, int top, int width, int height)
{
    if (regionArray.size() <= regionNumber)
        return;

    int begin = left + width + 10;

    scene->addLine(begin, top, 1.5 * begin, top);
    scene->addLine(begin, top + height, 1.5 * begin, top + height);

    regionArray.at(regionNumber).region = regionNumber;
}

void RegionInputData::setSpinBoxQuota(int regionNumber, int left, int top, int width, int height)
{
    QDoubleSpinBox *quote = new QDoubleSpinBox;

    quote->setValue(regionArray.at(regionNumber).quote);
    quote->setObjectName(QString::number(regionNumber));

    styleSpinBoxQuota(quote);

    QGraphicsProxyWidget *proxyWidget = new QGraphicsProxyWidget;
    proxyWidget->setWidget(quote);

    qreal rotationAngle = 90;
    proxyWidget->resize(height - 2, width - 2);
    proxyWidget->setRotation(rotationAngle);

    int begin = 2 * (left + width + 10);
    proxyWidget->setPos(begin, top + 2);

    scene->addItem(proxyWidget);

    connect( quote, &QDoubleSpinBox::valueChanged, this,
             [quote, this](double value)
    {
        bool ok = false;
        int quoteIndex = quote->objectName().toInt(&ok);
        regionArray.at(quoteIndex).quote = value;

        if (!ok)
            qWarning() << "Convertion QString to double error.";

        emit quote->destroyed(); // TBD - failure when using the wheel event. Issuing this signal avoids the error
        setGraphicScene(regionQuant);
    },
    Qt::SingleShotConnection);
}

void RegionInputData::styleSpinBoxQuota(QDoubleSpinBox *quota)
{
    QFont quoteFont("Arial", 9);

    quota->setDecimals(1);
    quota->setRange(2, 999);
    QLineEdit *lineEdit = quota->findChild<QLineEdit *>();
    lineEdit->setValidator(new QDoubleValidator(2, 999, 1, quota));
    quota->setFrame(false);
    quota->setSuffix("cm");
    quota->setFont(quoteFont);
    quota->setButtonSymbols(QAbstractSpinBox::NoButtons);
    quota->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    quota->setStyleSheet("background-color: white; "
                         "border: 0px solid black; border-bottom-width: 2px;"
                         "color: #00008B;");
}

void RegionInputData::loadGUI()
{
    if (!proj)
        proj = std::make_shared<ProjectData>();

    regionArray = std::move(proj->regionArray);

    this->updateGeometry();

    ui->spinBoxRegionQtt->setValue(proj->regionNumber);
    ui->spinBoxMaxNumberIteration->setValue(proj->maximumIterationsNumber);
    ui->spinBoxGroup->setValue(proj->energyGroup);
    ui->spinBoxQuadratureOrder->setValue(proj->quadratureOrder);

    scatteringPath = QString::fromStdString(proj->scatteringFilePath);

    QAbstractButton *bcLeftButton = ui->buttonGroupLeftBoundaryConditions->button(
                proj->leftBoundaryConditionsType);

    if (bcLeftButton)
        bcLeftButton->click();

    bcLeft.reset();

    if (proj->bcLeft.has_value()
        && proj->leftBoundaryConditionsType == ePrescribed)
    {
        bcLeft = proj->bcLeft.value();
    }

    QAbstractButton *bcRightButton = ui->buttonGroupRightBoundaryConditions->button(
                proj->rightBoundaryConditionsType);

    if (bcRightButton)
        bcRightButton->click();

    bcRight.reset();

    if (proj->bcRight.has_value()
        && proj->rightBoundaryConditionsType == ePrescribed)
    {
        bcRight = proj->bcRight.value();
    }

    ui->spinBoxStopOrder->setValue(proj->stopOrder);

    allZonasStr.clear();

    for (int iIndex = 0; iIndex < proj->regionNumber; ++iIndex)
    {
        if (!regionArray.empty() || regionArray.size() > iIndex)
        {
            auto zoneStr = QString::fromStdString(regionArray[iIndex].zoneStr);

            if (allZonasStr.contains(zoneStr))
                continue;

            allZonasStr.append(zoneStr);
        }
    }

    ui->spinBoxLegendreOrder->setValue(proj->legendreOrder);
}

void RegionInputData::saveGUI()
{
    if (!proj)
        proj = std::make_shared<ProjectData>();

    proj->regionArray = regionArray;
    proj->regionNumber = ui->spinBoxRegionQtt->value();
    proj->energyGroup = ui->spinBoxGroup->value();

    int leftBC = ui->buttonGroupLeftBoundaryConditions->checkedId();
    int rightBC = ui->buttonGroupRightBoundaryConditions->checkedId();

    if (leftBC == ePrescribed)
        proj->bcLeft = bcLeft;

    if (rightBC == ePrescribed)
        proj->bcRight = bcRight;

    proj->leftBoundaryConditionsType = eBoundaryConditionsType(leftBC);
    proj->rightBoundaryConditionsType = eBoundaryConditionsType(rightBC);
    proj->scatteringFilePath = scatteringPath.toStdString();
    proj->zoneNumber = allZonasStr.size();
    proj->legendreOrder = ui->spinBoxLegendreOrder->value();
    proj->quadratureOrder = ui->spinBoxQuadratureOrder->value();
    proj->maximumIterationsNumber = ui->spinBoxMaxNumberIteration->value();
    proj->stopOrder = ui->spinBoxStopOrder->value();
}

QStringList RegionInputData::getAllZonasStr() const
{
    return allZonasStr;
}

void RegionInputData::setPeriodicity(double newPeriodicity)
{
    periodicity = newPeriodicity;
}

void RegionInputData::setPushButtonCalculateFluxEnable(bool enable)
{
    ui->pushButtonCalculateFlux->setEnabled(enable);
}

std::shared_ptr<CalculatedData> RegionInputData::getDDOutputValues() const
{
    return DDOutputValues;
}

int RegionInputData::getRegionQuant() const
{
    return regionQuant;
}

int RegionInputData::getNumberOfGroup()
{
    return ui->spinBoxGroup->value();
}
