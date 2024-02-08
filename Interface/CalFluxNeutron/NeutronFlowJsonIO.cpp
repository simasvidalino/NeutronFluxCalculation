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
    //    saveFile.write(saveFormat == Interface::jsonFormat
    //                   ? QJsonDocument(obj).toJson()
    //                   : QCborValue::fromJsonValue(obj).toCbor());
    saveFile.write(QJsonDocument(obj).toJson());
}

void NeutronFlowJsonIO::loadProject(Interface::eSaveFormat saveFormat,
                                    QString path)
{
    QFile loadFile(path/*saveFormat == Interface::jsonFormat
                                                                                                                                           ? QStringLiteral("NeutronFlow.json")
                                                                                                                                           : QStringLiteral("NeutronFlow.dat")*/);

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

std::unique_ptr<Interface::projetData> &&NeutronFlowJsonIO::getGeneralProjectData()
{
    return std::move(generalProjectData);
}
void NeutronFlowJsonIO::setGeneralProjectData(std::unique_ptr<Interface::projetData> newGeneralProjectData)
{
    generalProjectData = std::move(newGeneralProjectData);
}

void NeutronFlowJsonIO::read(const QJsonObject &json)
{
    if (json.contains(GeneralProjectDataKey))
    {
        generalProjectData = std::make_unique<Interface::projetData>(
                    loadGeneralProjectData(json[GeneralProjectDataKey].toObject()));
    }

    if (json.contains(DataPerRegionKey))
    {
        QJsonArray regionObjArray = json[DataPerRegionKey].toArray();
        regionArray = loadRegionArray(regionObjArray);
        generalProjectData->regionArray = regionArray;
    }
}

void NeutronFlowJsonIO::write(QJsonObject &json) const
{
    if (generalProjectData)
        json[GeneralProjectDataKey] = saveGeneralProjectData();

    if (!regionArray.empty())
        json[DataPerRegionKey] = saveDataPerRegion();

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
        obj[ScatteringCrossSectionFileKey]  = generalProjectData->scateringFilePath.c_str();
        obj[StopOrderKey]                   = QString::number(generalProjectData->stopOrder);
        obj[RegionDataKey]                  = QString::number(generalProjectData->regionNumber);

        if (generalProjectData->bcLeft.has_value())
            obj[LeftBoundaryValuesKey] = saveObjArray(generalProjectData->bcLeft.value());

        if (generalProjectData->bcRight.has_value())
            obj[RightBoundaryValuesKey] = saveObjArray(generalProjectData->bcRight.value());    }

    return obj;
}

Interface::projetData NeutronFlowJsonIO::loadGeneralProjectData(const QJsonObject &obj)
{
    Interface::projetData projectData;
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
        projectData.energyGroup = obj[EnergyGroupKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Energy group conversion");
    }

    if (obj.contains(LeftBoundaryConditionsTypeKey))
    {
        projectData.leftBoundaryConditionsType = static_cast<Interface::eBoundaryConditionsType>(
                    obj[LeftBoundaryConditionsTypeKey].toString().toInt(&ok));

        verifyConversion(ok, "Error: Json Left boundary condition type conversion");
    }

    if (obj.contains(MaximumIterationsNumberKey))
    {
        projectData.maximumIterationsNumber = obj[MaximumIterationsNumberKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Maximum Iteration conversion");
    }

    if (obj.contains(RightBoundaryConditionsTypeKey))
    {
        projectData.rightBoundaryConditionsType = static_cast<Interface::eBoundaryConditionsType>(
                    obj[RightBoundaryConditionsTypeKey].toString().toInt(&ok));

        verifyConversion(ok, "Error: Json  Right boundary condition type conversion");
    }

    if (obj.contains(ScatteringCrossSectionFileKey))
        projectData.scateringFilePath = obj[ScatteringCrossSectionFileKey].toString().toStdString();

    if (obj.contains(QuadratureOrderKey))
    {
        projectData.quadratureOrder = obj[QuadratureOrderKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Quadrature Order conversion");
    }

    if (obj.contains(LegendreOrderKey))
    {
        projectData.legendreOrder = obj[LegendreOrderKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Legendre Order conversion");
    }

    if (obj.contains(StopOrderKey))
    {
        projectData.stopOrder = obj[StopOrderKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Stop Order conversion");
    }

    if (obj.contains(RegionDataKey))
    {
        projectData.regionNumber = obj[RegionDataKey].toString().toInt(&ok);
        verifyConversion(ok, "Error: Json Region Number conversion");
    }

    if (obj.contains(LeftBoundaryValuesKey))
    {
        auto arrayObj = obj[LeftBoundaryValuesKey].toArray();
        if (!projectData.bcLeft) {
            projectData.bcLeft = std::vector<double>(); // Inicializa o vector se ainda não foi inicializado
        }

        for (const auto& value:arrayObj)
        {
            projectData.bcLeft->push_back(value.toVariant().toDouble(&ok));
            verifyConversion(ok, "Error: Json left boundary condition conversion");
        }
    }

    if (obj.contains(RightBoundaryValuesKey))
    {
        auto arrayObj = obj[RightBoundaryValuesKey].toArray();

        if (!projectData.bcRight)
        {
            projectData.bcRight = std::vector<double>(); // Inicializa o vector se ainda não foi inicializado
        }

        for (const auto& value:arrayObj)
        {
            projectData.bcRight->push_back(value.toVariant().toDouble(&ok));
            verifyConversion(ok, "Error: Json left boundary condition conversion");
        }
    }

    return projectData;
}

QJsonArray NeutronFlowJsonIO::saveDataPerRegion() const
{
    QJsonArray regionJsonArray;

    for (int iIndex = 0; iIndex < generalProjectData->regionNumber; ++iIndex)
    {
        auto region = regionArray[iIndex];
        QJsonObject regionObj;
        QJsonArray physicalFontJsonArray;


        regionObj[NodeKey]          = region.node;
        regionObj[MaterialColorKey] = region.materialColor.name();
        regionObj[QuotaKey]         = region.quote;
        regionObj[ZoneStrKey]       = region.zoneStr;

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

std::array<Interface::regionData, 10> NeutronFlowJsonIO::loadRegionArray(const QJsonArray &objArray)
{
    std::array<Interface::regionData, 10> region;

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
            Interface::regionData regionData;
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
                    regionData.quote = obj[QuotaKey].toInt(50);
                else
                    verifyConversion(ok, "Error: Json Quota conversion");
            }

            if (obj.contains(ZoneStrKey))
            {
                regionData.zoneStr = obj[ZoneStrKey].toString();
            }

            if (obj.contains(MaterialColorKey))
            {
                regionData.materialColor = QColor(obj[MaterialColorKey].toString());
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

std::array<Interface::regionData, 10> NeutronFlowJsonIO::getRegionArray() const
{
    return regionArray;
}

void NeutronFlowJsonIO::setRegionArray(const std::array<Interface::regionData, 10> &newRegionArray)
{
    regionArray = newRegionArray;
}

