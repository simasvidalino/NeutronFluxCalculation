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
    return generalProjectData;
}

void NeutronFlowJsonIO::setGeneralProjectData(std::shared_ptr<ProjectData> newGeneralProjectData)
{
    generalProjectData = newGeneralProjectData;
}

void NeutronFlowJsonIO::read(const QJsonObject &json)
{
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
}

void NeutronFlowJsonIO::write(QJsonObject &json) const
{
    if (generalProjectData)
    {
        json[GeneralProjectDataKey] = saveGeneralProjectData();

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
        obj[ScatteringCrossSectionFileKey]  = generalProjectData->scatteringFilePath.c_str();
        obj[TotalScatteringCrossSectionFilePathKey]  = generalProjectData->totalScatteringCrossSectionFilePath.c_str();
        obj[AbsorptionCrossSectionFilePath]          = generalProjectData->absorptionCrossSectionFilePath.c_str();
        obj[StopOrderKey]                   = QString::number(generalProjectData->stopOrder);
        obj[RegionDataKey]                  = QString::number(generalProjectData->regionNumber);
        obj[PeriodicityKey]                 = generalProjectData->periodicity;
        obj[ZoneNumberKey]                  = generalProjectData->zoneNumber;

        obj[ScalarFluxFileKey]              = generalProjectData->scalarFluxFile.c_str();
        obj[AbsorptionRateFileKey]          = generalProjectData->absorptionRateFile.c_str();
        obj[AbsorptionRatePerNodeFileKey]   = generalProjectData->absorptionRatePerNodeFile.c_str();
        obj[AverageNeutronFluxPerRegionFileKey] = generalProjectData->averageNeutronFluxPerRegionFile.c_str();

        if (generalProjectData->bcLeft.has_value())
            obj[LeftBoundaryValuesKey] = saveObjArray(generalProjectData->bcLeft.value());

        if (generalProjectData->bcRight.has_value())
            obj[RightBoundaryValuesKey] = saveObjArray(generalProjectData->bcRight.value());    }

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

    if (obj.contains(ScatteringCrossSectionFileKey))
        ProjectData.scatteringFilePath = obj[ScatteringCrossSectionFileKey].toString().toStdString();

    if (obj.contains(TotalScatteringCrossSectionFilePathKey))
        ProjectData.totalScatteringCrossSectionFilePath = obj[TotalScatteringCrossSectionFilePathKey].toString().toStdString();

    if (obj.contains(AbsorptionCrossSectionFilePath))
        ProjectData.absorptionCrossSectionFilePath = obj[AbsorptionCrossSectionFilePath].toString().toStdString();

    if (obj.contains(ScalarFluxFileKey))
        ProjectData.scalarFluxFile = obj[ScalarFluxFileKey].toString().toStdString();

    if (obj.contains(AbsorptionRateFileKey))
        ProjectData.absorptionRateFile = obj[AbsorptionRateFileKey].toString().toStdString();

    if (obj.contains(AbsorptionRatePerNodeFileKey))
        ProjectData.absorptionRatePerNodeFile = obj[AbsorptionRatePerNodeFileKey].toString().toStdString();

    if (obj.contains(AverageNeutronFluxPerRegionFileKey))
        ProjectData.averageNeutronFluxPerRegionFile = obj[AverageNeutronFluxPerRegionFileKey].toString().toStdString();

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

    return ProjectData;
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

            qInfo()<<"load "<<obj;

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

std::array<RegionData, 10> NeutronFlowJsonIO::getRegionArray() const
{
    return generalProjectData->regionArray;
}

