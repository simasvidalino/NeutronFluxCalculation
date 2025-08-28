#include "NeutronFlowJsonIO.h"

#include <QCborValue>
#include <QCborMap>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

NeutronFlowJsonIO * NeutronFlowJsonIO::mClass = nullptr;

NeutronFlowJsonIO *NeutronFlowJsonIO::getInstance()
{
    if (mClass == nullptr)
        mClass = new NeutronFlowJsonIO;

    return mClass;
}

NeutronFlowJsonIO::NeutronFlowJsonIO()
{

}

void NeutronFlowJsonIO::saveProject(Interface::eSaveFormat saveFormat, QString path)
{
    bool ok = false;
    if (!path.contains(".json"))
        path.push_back(".json");

    QFile saveFile(path);

    if (!saveFile.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't save the file.");
        return;
    }

    QJsonObject obj;
    write(obj);
    saveFile.write(QJsonDocument(obj).toJson());
}

void NeutronFlowJsonIO::loadProject(Interface::eSaveFormat saveFormat,
                                    QString path)
{
    QFile loadFile(path);

    if (!loadFile.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't load the file.");
        return;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(saveFormat == Interface::jsonFormat
                          ? QJsonDocument::fromJson(saveData)
                          : QJsonDocument(QCborValue::fromCbor(saveData).toMap().toJsonObject()));

    read(loadDoc.object());

    QTextStream(stdout) << "Loaded save "
                        << " using "
                        << (saveFormat != Interface::jsonFormat ? "CBOR" : "JSON") << "...\n";
}

ProjectData *NeutronFlowJsonIO::getGeneralProjectDataPtr()
{
    return generalProjectData.get();
}

std::shared_ptr<ProjectData> NeutronFlowJsonIO::getGeneralProjectData()
{
    if (!generalProjectData)
        generalProjectData = std::make_shared<ProjectData>();

    return generalProjectData;
}

void NeutronFlowJsonIO::setGeneralProjectData(std::shared_ptr<ProjectData> newGeneralProjectData)
{
    generalProjectData = newGeneralProjectData;
}

void NeutronFlowJsonIO::read(const QJsonObject &json)
{
    calculatedData.reset();
    generalProjectData.reset();

    if (json.contains(GeneralProjectDataKey))
    {
        generalProjectData = std::make_shared<ProjectData>(
                    loadGeneralProjectData(json[GeneralProjectDataKey].toObject()));
    }

    if (json.contains(DataPerRegionKey))
    {
        QJsonArray regionObjArray = json[DataPerRegionKey].toArray();
        generalProjectData->regionArray = loadRegionArray(regionObjArray);;
    }

    if (json.contains(CalculationResultsKey))
    {
        calculatedData = std::make_shared<CalculatedData>(
            loadCalculatedData(json[CalculationResultsKey].toObject()));
    }
}

void NeutronFlowJsonIO::write(QJsonObject &json) const
{
    if (generalProjectData)
    {
        json[GeneralProjectDataKey] = saveGeneralProjectData();
        json[CalculationResultsKey] = saveCalculatedData();

        if (!generalProjectData->regionArray.empty())
            json[DataPerRegionKey] = saveDataPerRegion();
    }
}

QJsonObject NeutronFlowJsonIO::saveGeneralProjectData() const
{
    QJsonObject obj;

    auto saveObjArray = [](auto& vector)
    {
        QJsonArray mJsonArray;
        for (const auto& value:vector)
        {
            QJsonObject regionObj;
            mJsonArray.append(value);
        }

        return mJsonArray;
    };

    if (generalProjectData)
    {
        obj[EnergyGroupKey]                 = QString::number(generalProjectData->energyGroup);
        obj[QuadratureOrderKey]             = QString::number(generalProjectData->quadratureOrder);
        obj[LegendreOrderKey]               = QString::number(generalProjectData->legendreOrder);
        obj[MaximumIterationsNumberKey]     = QString::number(generalProjectData->maximumIterationsNumber);
        obj[LeftBoundaryConditionsTypeKey]  = QString::number(((int)generalProjectData->leftBoundaryConditionsType));
        obj[RightBoundaryConditionsTypeKey] = QString::number(((int)generalProjectData->rightBoundaryConditionsType));
        obj[StoppingCriteriaTypeKey]       = QString::number(((int)generalProjectData->stoppingCriteriaType));
        obj[ScatteringCrossSectionFileKey]  = generalProjectData->neutronMacroscopicCrossSectionsFilePath.c_str();
        obj[StopOrderKey]                   = QString::number(generalProjectData->stopOrder);
        obj[RegionDataKey]                  = QString::number(generalProjectData->regionNumber);
        obj[PeriodicityKey]                 = generalProjectData->periodicity;
        obj[ZoneNumberKey]                  = generalProjectData->zoneNumber;
        obj[PaletteKey]                     = generalProjectData->palette.c_str();
        obj[FontKey]                        = generalProjectData->font.c_str();
        obj[ScreenModeKey]                  = generalProjectData->screenMode.c_str();

        if (generalProjectData->bcLeft.has_value())
            obj[LeftBoundaryValuesKey] = saveObjArray(generalProjectData->bcLeft.value());

        if (generalProjectData->bcRight.has_value())
            obj[RightBoundaryValuesKey] = saveObjArray(generalProjectData->bcRight.value());
    }

    return obj;
}

ProjectData NeutronFlowJsonIO::loadGeneralProjectData(const QJsonObject &obj)
{
    ProjectData ProjectData;
    bool ok = false;

    auto loadArray = [&](QJsonArray& arrayObj, std::vector<double> &vector)
    {
        for (const auto& value:arrayObj)
            vector.push_back(value.toVariant().toDouble());

    };

    auto verifyConversion = [&](bool ok, const char* message){
        if (!ok)
            qWarning() << message;
    };

    if (obj.contains(EnergyGroupKey))
    {
        ProjectData.energyGroup = obj[EnergyGroupKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Energy group conversion");
    }

    if (obj.contains(LeftBoundaryConditionsTypeKey))
    {
        ProjectData.leftBoundaryConditionsType = static_cast<eBoundaryConditionsType>(
                    obj[LeftBoundaryConditionsTypeKey].toString().toInt(&ok));

        verifyConversion(ok, "Error: Json Left boundary condition type conversion");
    }

    if (obj.contains(MaximumIterationsNumberKey))
    {
        ProjectData.maximumIterationsNumber = obj[MaximumIterationsNumberKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Maximum Iteration conversion");
    }

    if (obj.contains(RightBoundaryConditionsTypeKey))
    {
        ProjectData.rightBoundaryConditionsType = static_cast<eBoundaryConditionsType>(
                    obj[RightBoundaryConditionsTypeKey].toString().toInt(&ok));

        verifyConversion(ok, "Error: Json  Right boundary condition type conversion");
    }

    if (obj.contains(StoppingCriteriaTypeKey))
    {
        ProjectData.stoppingCriteriaType = static_cast<eStoppingCriteriaType>(
            obj[StoppingCriteriaTypeKey].toString().toInt(&ok));

        verifyConversion(ok, "Error: Json  Stopping Criterion type conversion");
    }

    if (obj.contains(ScatteringCrossSectionFileKey))
        ProjectData.neutronMacroscopicCrossSectionsFilePath = obj[ScatteringCrossSectionFileKey].toString().toStdString();

    if (obj.contains(QuadratureOrderKey))
    {
        ProjectData.quadratureOrder = obj[QuadratureOrderKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Quadrature Order conversion");
    }

    if (obj.contains(LegendreOrderKey))
    {
        ProjectData.legendreOrder = obj[LegendreOrderKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Legendre Order conversion");
    }

    if (obj.contains(StopOrderKey))
    {
        ProjectData.stopOrder = obj[StopOrderKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Stop Order conversion");
    }

    if (obj.contains(RegionDataKey))
    {
        ProjectData.regionNumber = obj[RegionDataKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Region Number conversion");
    }

    if (obj.contains(PeriodicityKey))
    {
        if (!obj[PeriodicityKey].isDouble())
            verifyConversion(false, "Error: Json Periodicity conversion");

        ProjectData.periodicity = obj[PeriodicityKey].toDouble(10);

    }

    if (obj.contains(ZoneNumberKey))
    {
        if (!obj[ZoneNumberKey].isDouble())
            verifyConversion(false, "Error: Json Zone Number conversion");

        ProjectData.zoneNumber = obj[ZoneNumberKey].toInt();
    }

    if (obj.contains(LeftBoundaryValuesKey))
    {
        auto arrayObj = obj[LeftBoundaryValuesKey].toArray();
        if (!ProjectData.bcLeft) {
            ProjectData.bcLeft = std::vector<double>(); // Inicializa o vector se ainda não foi inicializado
        }

        for (const auto& value:arrayObj)
        {
            ProjectData.bcLeft->push_back(value.toVariant().toDouble(&ok));
            verifyConversion(ok, "Error: Json left boundary condition conversion");
        }
    }

    if (obj.contains(RightBoundaryValuesKey))
    {
        auto arrayObj = obj[RightBoundaryValuesKey].toArray();

        if (!ProjectData.bcRight)
        {
            ProjectData.bcRight = std::vector<double>(); // Inicializa o vector se ainda não foi inicializado
        }

        for (const auto& value:arrayObj)
        {
            ProjectData.bcRight->push_back(value.toVariant().toDouble(&ok));
            verifyConversion(ok, "Error: Json left boundary condition conversion");
        }
    }

    if (obj.contains(PaletteKey))
        ProjectData.palette = obj[PaletteKey].toString().toStdString();

    if (obj.contains(FontKey))
        ProjectData.font = obj[FontKey].toString().toStdString();

    if (obj.contains(ScreenModeKey))
        ProjectData.screenMode = obj[ScreenModeKey].toString().toStdString();

    return ProjectData;
}

QJsonObject NeutronFlowJsonIO::saveCalculatedData() const
{
    QJsonObject obj;

    if (calculatedData && generalProjectData)
    {
        const auto& regionArray                       = generalProjectData->regionArray;
        int nodex                                     = 0;
        double totalRegionSize                        = 0;

        obj[TotalScatteringCrossSectionFilePathKey]   = calculatedData->matrices.totalScatteringCrossSectionFile.c_str();
        obj[AbsorptionCrossSectionFilePath]           = calculatedData->matrices.absorptionCrossSectionFile.c_str();
        obj[ScalarFluxFileKey]                        = calculatedData->scalarFluxFile.c_str();
        obj[AverageAbsorptionRatePerRegionFileKey]    = calculatedData->averageAbsorptionRatePerRegionFile.c_str();
        obj[AbsorptionRatePerNodeFileKey]             = calculatedData->absorptionRatePerNodeFile.c_str();
        obj[AverageNeutronFluxPerRegionFileKey]       = calculatedData->averageNeutronFluxPerRegionFile.c_str();
        obj[IntegratedNeutronFluxPerRegionFileKey]    = calculatedData->integratedNeutronFluxPerRegionFile.c_str();
        obj[IntegratedAbsorptionRatePerRegionFileKey] = calculatedData->integratedAbsorptionRatePerRegionFile.c_str();

        for (int rIndex = 0; rIndex < generalProjectData->regionNumber; ++rIndex)
        {
            totalRegionSize += regionArray[rIndex].quote;
            nodex += regionArray[rIndex].node;
        }

        obj[NeutronFluxPointsPerNodeKey]          = saveArrayPerNode(generalProjectData->energyGroup,
                                                            nodex,
                                                            totalRegionSize,
                                                            NeutronFluxPointsPerNodeKey,
                                                            calculatedData->scalarFlux);

        obj[AbsorptionRatePerNodeKey]             = saveArrayPerNode(generalProjectData->energyGroup,
                                                         nodex,
                                                         totalRegionSize,
                                                         AbsorptionRatePerNodeKey,
                                                         calculatedData->absorptionRatePerNode);

        obj[AverageNeutronFluxPointsPerRegion]    = saveArrayPerGroupPerRegion(generalProjectData->regionNumber,
                                                                            generalProjectData->energyGroup,
                                                                            AverageNeutronFluxPointsPerRegion,
                                                                            calculatedData->averageNeutronFluxPerRegion);

        obj[AverageAbsorptionRatePerRegionKey]    = saveArrayPerGroupPerRegion(generalProjectData->regionNumber,
                                                                            generalProjectData->energyGroup,
                                                                            AverageAbsorptionRatePerRegionKey,
                                                                            calculatedData->averageAbsorptionRatePerRegion);

        obj[IntegratedAbsorptionRatePerRegionKey] = saveArrayPerGroupPerRegion(generalProjectData->regionNumber,
                                                                               generalProjectData->energyGroup,
                                                                               IntegratedAbsorptionRatePerRegionKey,
                                                                               calculatedData->integratedAbsorptionRatePerGroupPerRegion);

        obj[TotalAbsorptionRatePerGroupPerRegionKey] =
            saveArrayPerGroupPerRegion(generalProjectData->regionNumber,
                                       generalProjectData->energyGroup,
                                       TotalAbsorptionRatePerGroupPerRegionKey,
                                       calculatedData->totalAbsorptionRatePerGroupPerRegion);

        obj[TotalScalarNeutronFluxPerGroupPerRegionKey] =
            saveArrayPerGroupPerRegion(generalProjectData->regionNumber,
                                       generalProjectData->energyGroup,
                                       TotalScalarNeutronFluxPerGroupPerRegionKey,
                                       calculatedData->totalNeutronFluxPerGroupPerRegion);

        obj[TotalAbsorptionRatePerRegionKey]    = saveArrayPerRegion(generalProjectData->regionNumber,
                                                                  TotalAbsorptionRatePerRegionKey,
                                                                  calculatedData->totalAbsorptionRatePerRegion);

        obj[TotalScalarNeutronFluxPerRegionKey] = saveArrayPerRegion(generalProjectData->regionNumber,
                                                                     TotalScalarNeutronFluxPerRegionKey,
                                                                     calculatedData->totalNeutronFluxPerRegion);
    }

    return obj;
}

CalculatedData NeutronFlowJsonIO::loadCalculatedData(const QJsonObject& obj)
{
    CalculatedData calculatedData;

    if (obj.contains(TotalScatteringCrossSectionFilePathKey))
        calculatedData.matrices.totalScatteringCrossSectionFile = obj[TotalScatteringCrossSectionFilePathKey].toString().toStdString();

    if (obj.contains(AbsorptionCrossSectionFilePath))
        calculatedData.matrices.absorptionCrossSectionFile = obj[AbsorptionCrossSectionFilePath].toString().toStdString();

    if (obj.contains(ScalarFluxFileKey))
        calculatedData.scalarFluxFile = obj[ScalarFluxFileKey].toString().toStdString();

    if (obj.contains(AverageAbsorptionRatePerRegionFileKey))
        calculatedData.averageAbsorptionRatePerRegionFile = obj[AverageAbsorptionRatePerRegionFileKey].toString().toStdString();

    if (obj.contains(AbsorptionRatePerNodeFileKey))
        calculatedData.absorptionRatePerNodeFile = obj[AbsorptionRatePerNodeFileKey].toString().toStdString();

    if (obj.contains(AverageNeutronFluxPerRegionFileKey))
        calculatedData.averageNeutronFluxPerRegionFile = obj[AverageNeutronFluxPerRegionFileKey].toString().toStdString();

    if (obj.contains(IntegratedNeutronFluxPerRegionFileKey))
        calculatedData.integratedNeutronFluxPerRegionFile = obj[IntegratedNeutronFluxPerRegionFileKey].toString().toStdString();

    if (obj.contains(IntegratedAbsorptionRatePerRegionFileKey))
        calculatedData.integratedAbsorptionRatePerRegionFile = obj[IntegratedAbsorptionRatePerRegionFileKey].toString().toStdString();

    if (obj.contains(NeutronFluxPointsPerNodeKey))
        calculatedData.scalarFlux = loadArrayPerNode(obj[NeutronFluxPointsPerNodeKey].toArray());

    if (obj.contains(AbsorptionRatePerNodeKey))
        calculatedData.absorptionRatePerNode = loadArrayPerNode(obj[AbsorptionRatePerNodeKey].toArray());

    if (obj.contains(AverageNeutronFluxPointsPerRegion))
        calculatedData.averageNeutronFluxPerRegion = loadArrayPerGroupPerRegion(obj[AverageNeutronFluxPointsPerRegion].toArray());

    if (obj.contains(AverageAbsorptionRatePerRegionKey))
        calculatedData.averageAbsorptionRatePerRegion = loadArrayPerGroupPerRegion(obj[AverageAbsorptionRatePerRegionKey].toArray());

    if (obj.contains(IntegratedAbsorptionRatePerRegionKey))
        calculatedData.integratedAbsorptionRatePerGroupPerRegion = loadArrayPerGroupPerRegion(obj[IntegratedAbsorptionRatePerRegionKey].toArray());

    if (obj.contains(TotalAbsorptionRatePerGroupPerRegionKey))
        calculatedData.totalAbsorptionRatePerGroupPerRegion = loadArrayPerGroupPerRegion(obj[TotalAbsorptionRatePerGroupPerRegionKey].toArray());

    if (obj.contains(TotalScalarNeutronFluxPerGroupPerRegionKey))
        calculatedData.totalNeutronFluxPerGroupPerRegion = loadArrayPerGroupPerRegion(obj[TotalScalarNeutronFluxPerGroupPerRegionKey].toArray());

    if (obj.contains(TotalAbsorptionRatePerRegionKey))
        calculatedData.totalAbsorptionRatePerRegion = loadArrayPerRegion(obj[TotalAbsorptionRatePerRegionKey].toArray());

    if (obj.contains(TotalScalarNeutronFluxPerRegionKey))
        calculatedData.totalNeutronFluxPerRegion = loadArrayPerRegion(obj[TotalScalarNeutronFluxPerRegionKey].toArray());

    return calculatedData;
}

QJsonArray NeutronFlowJsonIO::saveDataPerRegion() const
{
    QJsonArray regionJsonArray;

    auto& regionArray = generalProjectData->regionArray;

    for (int iIndex = 0; iIndex < generalProjectData->regionNumber; ++iIndex)
    {
        auto region = regionArray[iIndex];
        QJsonObject regionObj;
        QJsonArray physicalFontJsonArray;

        regionObj[NodeKey]          = region.node;
        regionObj[MaterialColorKey] = region.materialColor;
        regionObj[QuotaKey]         = region.quote;
        regionObj[ZoneNumberKey]    = region.zone;
        regionObj[ZoneStrKey]       = region.zoneStr.c_str();

        if (region.physicalSource.has_value())
        {
            for (const auto valuePerGroup : region.physicalSource.value())
                physicalFontJsonArray.push_back(valuePerGroup);

            regionObj[PhysicalKey] = physicalFontJsonArray;
        }

        regionJsonArray.append(regionObj);
    }

    return regionJsonArray;
}

std::array<RegionData, 10> NeutronFlowJsonIO::loadRegionArray(const QJsonArray &objArray)
{
    std::array<RegionData, 10> region;

    auto verifyConversion = [&](bool ok, const char* message){
        if (!ok)
            qWarning() << message;
    };

    int iIndex = 0;
    for (const auto value : objArray)
    {
        if (value.isObject())
        {
            auto obj = value.toObject();
            RegionData regionData;
            bool ok = false;

            if (obj.contains(NodeKey))
            {
                if (obj[NodeKey].isDouble())
                    regionData.node = obj[NodeKey].toInt(20);
                else
                    verifyConversion(ok, "Error: Json Node conversion");
            }

            if (obj.contains(QuotaKey))
            {
                if (obj[QuotaKey].isDouble())
                    regionData.quote = obj[QuotaKey].toDouble(50);
                else
                    verifyConversion(ok, "Error: Json Quota conversion");
            }

            if (obj.contains(ZoneNumberKey))
            {
                regionData.zone = obj[ZoneNumberKey].toInt();
            }

            if (obj.contains(ZoneStrKey))
            {
                regionData.zoneStr = obj[ZoneStrKey].toString().toStdString();
            }

            if (obj.contains(MaterialColorKey))
            {
                regionData.materialColor = obj[MaterialColorKey].toInt();
            }

            if (obj.contains(PhysicalKey))
            {
                QJsonArray physicalFontArray = obj[PhysicalKey].toArray();
                std::vector<double> physivalFontVector;

                for (const auto & physivalKeyByGroup : physicalFontArray)
                    physivalFontVector.push_back(physivalKeyByGroup.toDouble());

                regionData.physicalSource = physivalFontVector;
            }

            region[iIndex] = regionData;

            ++iIndex;
        }
    }

    return region;
}

QJsonArray NeutronFlowJsonIO::saveArrayPerNode(int group,
                                               int nodex,
                                               int totalRegionSize,
                                               const char *key,
                                               std::vector<std::vector<long double>>& vectorData) const
{
    QJsonArray finalJsonArray;

    if (vectorData.size() == 0)
    {
        return finalJsonArray;
    }

    double stepSize = totalRegionSize / static_cast<double>(nodex);

    for (int gIndex = 0; gIndex < group; ++gIndex)
    {
        QJsonObject byEnergyObj;
        QJsonArray valuesArray;
        double positionX = 0.0;

        byEnergyObj[EnergyGroupKey] = gIndex + 1;

        for (int nod = 0; nod < nodex; ++nod)
        {
            QJsonObject byPositionObj;

            long double value = vectorData[gIndex][nod];

            byPositionObj[PositionKey] = positionX;
            byPositionObj[key] = static_cast<double>(value);

            valuesArray.append(byPositionObj);

            positionX += stepSize;
        }

        byEnergyObj[ValuesKey] = valuesArray;
        finalJsonArray.append(byEnergyObj);
    }

    return finalJsonArray;
}

std::vector<std::vector<long double>> NeutronFlowJsonIO::loadArrayPerNode(const QJsonArray& objArray)
{
    std::vector<std::vector<long double>> result;

    for (const QJsonValue& groupVal : objArray)
    {
        if (!groupVal.isObject())
        {
            continue;
        }

        QJsonObject groupObj = groupVal.toObject();

        if (!groupObj.contains(ValuesKey))
        {
            continue;
        }

        QJsonArray valuesArray = groupObj[ValuesKey].toArray();

        std::vector<long double> groupData;

        for (const QJsonValue& posVal : valuesArray)
        {
            if (!posVal.isObject())
            {
                continue;
            }

            QJsonObject posObj = posVal.toObject();

            if (posObj.contains(PositionKey))
            {
                for (auto it = posObj.begin(); it != posObj.end(); ++it)
                {
                    if (it.key() != PositionKey)
                    {
                        groupData.push_back(it.value().toDouble());
                        break;
                    }
                }
            }
        }

        result.push_back(groupData);
    }

    return result;
}

QJsonArray NeutronFlowJsonIO::saveArrayPerGroupPerRegion(int regionNumber,
                                                 int group,
                                                 const char *,
                                                 std::vector<std::vector<long double>>& vectorData) const
{
    QJsonArray finalJsonArray;

    if (vectorData.empty())
    {
        return finalJsonArray;
    }

    for (int rIndex = 0; rIndex < regionNumber; ++rIndex)
    {
        QJsonObject byRegionObj;
        QJsonArray valuesArray;

        byRegionObj[RegionKey] = rIndex + 1;

        for (int gIndex = 0; gIndex < group; ++gIndex)
        {
            long double value = vectorData[rIndex][gIndex];
            valuesArray.append(static_cast<double>(value));
        }

        byRegionObj[ValuesKey] = valuesArray;
        finalJsonArray.append(byRegionObj);
    }

    return finalJsonArray;
}


std::vector<std::vector<long double>> NeutronFlowJsonIO::loadArrayPerGroupPerRegion(const QJsonArray& objArray)
{
    std::vector<std::vector<long double>> result;

    for (const QJsonValue& regionVal : objArray)
    {
        if (!regionVal.isObject())
            continue;

        QJsonObject regionObj = regionVal.toObject();

        if (!regionObj.contains(ValuesKey))
            continue;

        QJsonArray valuesArray = regionObj[ValuesKey].toArray();

        std::vector<long double> regionData;

        for (const QJsonValue& val : valuesArray)
        {
            regionData.push_back(val.toDouble());
        }

        result.push_back(regionData);
    }

    return result;
}

QJsonArray NeutronFlowJsonIO::saveArrayPerRegion(int regionNumber,
                                                 const char* key,
                                                 std::vector<long double>& vectorData) const
{
    QJsonArray finalJsonArray;

    if (vectorData.empty())
    {
        return finalJsonArray;
    }

    for (int rIndex = 0; rIndex < regionNumber; ++rIndex)
    {
        QJsonObject obj;
        obj[RegionKey] = rIndex + 1;
        obj[ValueKey] = static_cast<double>(vectorData[rIndex]);
        finalJsonArray.append(obj);
    }

    return finalJsonArray;
}

std::vector<long double> NeutronFlowJsonIO::loadArrayPerRegion(const QJsonArray& objArray)
{
    std::vector<long double> result;

    for (const QJsonValue& v : objArray)
    {
        if (!v.isObject())
        {
            continue;
        }

        const QJsonObject o  = v.toObject();
        const double val = o[ValueKey].toDouble();

        result.push_back(static_cast<long double>(val));
    }

    return result;
}

std::shared_ptr<CalculatedData> NeutronFlowJsonIO::getCalculatedData()
{
    return calculatedData;
}

void NeutronFlowJsonIO::setCalculatedData(const std::shared_ptr<CalculatedData>& newCalculatedData)
{
    calculatedData = newCalculatedData;
}

std::array<RegionData, 10> NeutronFlowJsonIO::getRegionArray() const
{
    return generalProjectData->regionArray;
}

