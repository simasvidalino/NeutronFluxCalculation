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

    if (json.contains(RegionDataKey) && json[RegionDataKey].isArray())
    {
        QJsonArray regionArray = json[RegionDataKey].toArray();
        dataPerRegion.clear();
        dataPerRegion = loadDataPerRegion(regionArray);
    }
}

void NeutronFlowJsonIO::write(QJsonObject &json) const
{
    if (generalProjectData)
        json[GeneralProjectDataKey] = saveGeneralProjectData();

    if (!dataPerRegion.empty())
        json[DataPerRegionKey] = saveDataPerRegion();

}

std::vector<std::shared_ptr<Interface::regionData> > NeutronFlowJsonIO::getDataPerRegion()
{
    return std::move(dataPerRegion);
}

void NeutronFlowJsonIO::setDataPerRegion(const std::vector<std::shared_ptr<Interface::regionData> > &newDataPerRegion)
{
    dataPerRegion = std::move(newDataPerRegion);
}

QJsonObject NeutronFlowJsonIO::saveGeneralProjectData() const
{
    QJsonObject obj;

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

    }

    return obj;
}

Interface::projetData NeutronFlowJsonIO::loadGeneralProjectData(const QJsonObject &obj)
{
    Interface::projetData projectData;
    bool ok = false;

    if (obj.contains(EnergyGroupKey)) {
        projectData.energyGroup = obj[EnergyGroupKey].toString().toInt(&ok);
    }

    if (obj.contains(LeftBoundaryConditionsTypeKey))
    {
        projectData.leftBoundaryConditionsType = static_cast<Interface::eBoundaryConditionsType>(
                    obj[LeftBoundaryConditionsTypeKey].toString().toInt(&ok));
    }

    if (obj.contains(MaximumIterationsNumberKey))
    {
        projectData.maximumIterationsNumber = obj[MaximumIterationsNumberKey].toString().toInt(&ok);
    }

    if (obj.contains(RightBoundaryConditionsTypeKey))
    {
        projectData.rightBoundaryConditionsType = static_cast<Interface::eBoundaryConditionsType>(
                    obj[RightBoundaryConditionsTypeKey].toString().toInt(&ok));
    }

    if (obj.contains(ScatteringCrossSectionFileKey))
    {
        projectData.scateringFilePath = obj[ScatteringCrossSectionFileKey].toString().toStdString();
    }

    if (obj.contains(QuadratureOrderKey))
    {
        projectData.quadratureOrder = obj[QuadratureOrderKey].toString().toInt(&ok);
    }

    if (obj.contains(LegendreOrderKey))
    {
        projectData.legendreOrder = obj[LegendreOrderKey].toString().toInt(&ok);
    }

    if (obj.contains(StopOrderKey))
    {
        projectData.stopOrder = obj[StopOrderKey].toString().toInt(&ok);
    }

    if (obj.contains(RegionDataKey))
    {
        projectData.regionNumber = obj[RegionDataKey].toString().toInt(&ok);
    }

    if (!ok)
         qWarning() << "NeutronFlowJsonIO: Failed to convert types";

    return projectData;
}

QJsonArray NeutronFlowJsonIO::saveDataPerRegion() const
{
    QJsonArray regionArray;

    for (const auto& region:dataPerRegion)
    {
        if (region)
        {
            QJsonObject regionObj;
            regionObj[NodeKey] = region->node;
            regionObj[MaterialColorKey] = region->materialColor.name();
            regionObj[QuotaKey] = region->quote;
            regionObj[ZoneStrKey] = region->zoneStr;
        }
    }

    return regionArray;
}
std::vector<std::shared_ptr<Interface::regionData>> NeutronFlowJsonIO::loadDataPerRegion(
        const QJsonArray &objArray)
{
    std::vector<std::shared_ptr<Interface::regionData>> region;
    bool ok = false;

    for (const auto value : objArray)
    {
        if (value.isObject())
        {
            auto obj = value.toObject();
            auto regionData = std::make_shared<Interface::regionData>();

            if (obj.contains(NodeKey))
                regionData->node = obj[NodeKey].toString().toInt(&ok);

            dataPerRegion.push_back(regionData);
        }
    }

    if (!ok)
         qWarning() << "NeutronFlowJsonIO: Failed to convert types";

    return region;
}

