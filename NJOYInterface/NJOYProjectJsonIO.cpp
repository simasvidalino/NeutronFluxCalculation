#include "NJOYProjectJsonIO.h"
#include "qjsonarray.h"

#include <QCborValue>
#include <QCborMap>
#include <QJsonDocument>
#include <QFile>

NJOYProjectJsonIO* NJOYProjectJsonIO::m_class = nullptr;

NJOYProjectJsonIO::NJOYProjectJsonIO() :
    generalParameters(nullptr),
    choosingIsotopes(nullptr),
    choosingModules(nullptr)
{

}

QJsonObject NJOYProjectJsonIO::saveChoosingModules() const
{
    QJsonObject obj;

    obj[MODERModuleKey] = saveModer();

    obj[GROUPRModuleKey] = saveGrouprInput();

    obj[RECONRModuleKey] = saveReconrInput();

    obj[UNRESRModuleKey] = saveUnresrInput();

    return obj;
}

std::unique_ptr<NJOYChoosingModulesJsonIO> NJOYProjectJsonIO::loadChoosingModules(const QJsonObject &obj)
{
    auto ci = std::make_unique<NJOYChoosingModulesJsonIO>();

    if (obj.contains(MODERModuleKey))
    {
        auto test = obj[ChoosingModulesKey];
        const auto moderObj = obj[MODERModuleKey].toObject();
        ci->setModer(loadModer(moderObj));
    }

    if (obj.contains(GROUPRModuleKey))
    {
        auto grouprArray = obj[GROUPRModuleKey].toArray();

        const auto grouprVect = loadGrouprInput(grouprArray);
        for (auto i = 0; i < grouprArray.size(); ++i)
        {
            auto ptr = std::make_unique<NJOYGrouprInput>(*grouprVect[i]);
            ci->setGroupr(std::move(ptr));
        }
    }

    if (obj.contains(RECONRModuleKey))
    {
        auto reconrArray = obj[RECONRModuleKey].toArray();

        const auto reconrVect = loadReconrInput(reconrArray);
        for (auto i = 0; i < reconrVect.size(); ++i)
        {
            auto ptr = std::make_unique<NJOYReconrInput>(*reconrVect[i]);
            ci->setReconr(std::move(ptr));
        }
    }

    if (obj.contains(UNRESRModuleKey))
    {
        auto unresrArray = obj[UNRESRModuleKey].toArray();

        const auto unresrVect = loadUnresrInput(unresrArray);
        for (auto i = 0; i < unresrArray.size(); ++i)
        {
            auto ptr = std::make_unique<NJOYUnresInput>(*unresrVect[i]);
            ci->setUnresr(std::move(ptr));
        }
    }
    return std::move(ci);
}

QJsonObject NJOYProjectJsonIO::saveChoosingIsotopes() const
{
    QJsonObject obj;
    QJsonArray fileNames;

    std::for_each(choosingIsotopes->fileNames.begin(), choosingIsotopes->fileNames.end(), [&](std::string path){
        fileNames.append(path.c_str());
    });

    obj[InputFileKey] = fileNames;

    return obj;
}

NJOYChoosingIsotopes NJOYProjectJsonIO::loadChoosingIsotopes(const QJsonObject &obj)
{
    NJOYChoosingIsotopes ci;

    if (obj.contains(InputFileKey))
    {
        auto array = obj[InputFileKey].toArray();

        std::for_each(array.begin(), array.end(), [&](const auto path){
            ci.fileNames.push_back(path.toString().toStdString());});
    }

    return ci;
}

QJsonObject NJOYProjectJsonIO::saveGeneralParameters() const
{
    QJsonObject obj;

    obj[MatNumberKey]    = QString::fromStdString(generalParameters->matNumber);
    obj[ReactionKey]     = QString::fromStdString(generalParameters->reactions);
    obj[TemperaturesKey] = QString::fromStdString(generalParameters->temperatures);

    return obj;
}

NJOYGeneralParametersInput NJOYProjectJsonIO::loadGeneralParametersInput(const QJsonObject &obj)
{
    NJOYGeneralParametersInput gp;

    if (obj.contains(MatNumberKey))
    {
        gp.matNumber = obj[MatNumberKey].toString().toStdString();
    }

    if (obj.contains(ReactionKey))
    {
        gp.reactions = obj[ReactionKey].toString().toStdString();
    }

    if (obj.contains(TemperaturesKey))
    {
        gp.temperatures = obj[TemperaturesKey].toString().toStdString();
    }

    return gp;
}

QJsonArray NJOYProjectJsonIO::saveGrouprInput() const
{
    QJsonArray array;

    const auto grouprVect = choosingModules->getGroupr();

    auto convertVectoroArrayObj  = [&](const auto vector)
    {
        QJsonArray array;

        for (const auto &value : vector)
            array.append(QString::number(value, 'g', 15));

        return array;
    };

    for (const auto & value : grouprVect)
    {
        QJsonObject obj;

        obj[TitleKey]         = QString::fromStdString(value->title);
        obj[MatNumberKey]     = QString::fromStdString(value->matNumber);
        obj[NeutronGroupStructureKey] = QString::number(value->neutronGroupStructure);
        obj[LegendreOrderKey] = QString::number(value->legendreOrder);
        obj[SmoothOptionKey]  = QString::number(value->smoothOption);

        if (value->energyGammaGroups.has_value())
        {
            obj[NumberOfEnergyGammaKey] = QString::number(value->energyGammaGroups->size());

            const std::vector<long double> values = value->energyGammaGroups.value();

            obj[EnergyGammaKey] = convertVectoroArrayObj(values);
        }

        if (value->energyNeutronGroups.has_value())
        {
            obj[NumberOfEnergyNeutronKey] = QString::number(value->energyNeutronGroups->size());

            const std::vector<long double> values = value->energyNeutronGroups.value();

            obj[EnergyNeutronKey] = convertVectoroArrayObj(values);
        }

        if (value->fileAndSectionTobeProcessed.has_value())
        {
            //TBD
            value->fileAndSectionTobeProcessed.value();
        }

        if (value->numberOfNeutronGroups.has_value())
        {
            obj[NumberOfNeutronGroupsKey] = QString::number(value->numberOfNeutronGroups.value());
        }

        const auto sigmaZero = value->sigmaZeroValues;

        obj[SigmasZeroKey] = convertVectoroArrayObj(sigmaZero);

        obj[NumberOfSigmasZeroKey] = QString::number(sigmaZero.size());

        obj[MTDOptionKey] = QString::number(value->MTDOption);

        obj[LongPrintOptionKey] = QString::number(value->longPrintOption);

        const auto temperatures = value->temperatures;

        obj[NumberOfTemperaturesKey] = QString::number(temperatures.size());

        obj[TemperaturesKey] = convertVectoroArrayObj(temperatures);

        array.append(obj);
    }

    return array;
}

std::vector<std::unique_ptr<NJOYGrouprInput> > NJOYProjectJsonIO::loadGrouprInput(const QJsonArray &array)
{
    std::vector<std::unique_ptr<NJOYGrouprInput>> groupr;

    auto convertArrayObjToVector  = [&](const auto arrayObj, auto &vector)
    {
        for (const auto &value : arrayObj)
            vector.push_back(value.toDouble());

        return vector;
    };

    std::for_each(array.begin(), array.end(), [&](const QJsonValue item)
    {
        if (item.isObject())
        {
            QJsonObject obj(item.toObject());
            auto grouprPtr = std::make_unique<NJOYGrouprInput>();

            if (obj.contains(TitleKey))
                grouprPtr->title = obj[TitleKey].toString().toStdString();

            if (obj.contains(MatNumberKey))
                grouprPtr->matNumber = obj[MatNumberKey].toInt();

            if (obj.contains(NeutronGroupStructureKey))
                grouprPtr->neutronGroupStructure = obj[NeutronGroupStructureKey].toInt();

            if (obj.contains(LegendreOrderKey))
                grouprPtr->legendreOrder = obj[LegendreOrderKey].toInt();

            if (obj.contains(SmoothOptionKey))
                grouprPtr->smoothOption = obj[SmoothOptionKey].toInt();

            if (obj.contains(NumberOfEnergyGammaKey))
            {
                grouprPtr->numberOfGammaGroup = obj[NumberOfEnergyGammaKey].toInt();

                std::vector<long double> vector;
                convertArrayObjToVector(obj[EnergyGammaKey].toArray(), vector);
                grouprPtr->energyGammaGroups = vector;
            }

            if (obj.contains(NumberOfEnergyNeutronKey))
            {
                grouprPtr->numberOfNeutronGroups = obj[NumberOfEnergyNeutronKey].toInt();

                std::vector<long double> vector;
                convertArrayObjToVector(obj[EnergyNeutronKey].toArray(), vector);
                grouprPtr->energyNeutronGroups = vector;
            }

            //if (obj.contains(Fil))
                //grouprPtr->fileAndSectionTobeProcessed

            if (obj.contains(SigmasZeroKey))
            {
                std::vector<double> vector;
                convertArrayObjToVector(obj[SigmasZeroKey].toArray(), vector);
                grouprPtr->sigmaZeroValues = vector;

                grouprPtr->numberOfSigmaZeros = vector.size();
            }

            if (obj.contains(MTDOptionKey))
                grouprPtr->MTDOption = obj[MTDOptionKey].toInt();

            if (obj.contains(LongPrintOptionKey))
                grouprPtr->longPrintOption = obj[LongPrintOptionKey].toInt();

            if (obj.contains(NumberOfTemperaturesKey))
            {
                std::vector<double> vector;
                convertArrayObjToVector(obj[TemperaturesKey].toArray(), vector);
                grouprPtr->temperatures = vector;

                grouprPtr->numberOfTemperatures = vector.size();
            }

            groupr.push_back(std::move(grouprPtr));
        }

    });

    return std::move(groupr);
}

QJsonObject NJOYProjectJsonIO::saveModer() const
{
    QJsonObject obj;
    QJsonArray inputTapesAndMatArray;

    auto moder = choosingModules->getModer();

    obj[InputFileKey]   = QString::fromStdString(moder->input);
    obj[OutputFileKey]  = QString::fromStdString(moder->output);

    if (moder->inputTapesAndMat.has_value())
    {
        auto  i = moder->inputTapesAndMat.value();

        std::for_each(i.begin(), i.end(), [&](const auto pair){
            QJsonObject jsonObject;

            QString key = QString::fromStdString(pair.first);
            QString value = QString::fromStdString(pair.second);
            jsonObject[key] = value;

            inputTapesAndMatArray.append(jsonObject);
        });

        obj[InputTapesAndMatKey] = inputTapesAndMatArray;
    }

    return obj;
}
std::unique_ptr<NJOYModerInput> NJOYProjectJsonIO::loadModer(const QJsonObject &obj)
{
    auto moder = std::make_unique<NJOYModerInput>();

    if (obj.contains(InputFileKey))
    {
        moder->input = obj[InputFileKey].toString().toStdString();
    }

    if (obj.contains(OutputFileKey))
    {
        moder->output = obj[OutputFileKey].toString().toStdString();
    }

    if (obj.contains(InputTapesAndMatKey))
    {
        auto array = obj[InputTapesAndMatKey].toArray();
        std::map<std::string, std::string> map;

        for (const auto value : array)
        {
            QJsonObject fileAndMatObject = value.toObject();

            for (auto it = fileAndMatObject.begin(); it != fileAndMatObject.end(); ++it)
            {
                const auto key = it.key().toStdString();
                const auto value = it.value().toString().toStdString();

                map[key] = value;
            }
\
        }

        moder->inputTapesAndMat = map;
    }

    return std::move(moder);
}

NJOYProjectJsonIO *NJOYProjectJsonIO::getInstance()
{
    if (m_class == nullptr)
        m_class = new NJOYProjectJsonIO;

    return m_class;
}

void NJOYProjectJsonIO::deleteAllModules()
{
    choosingModules.reset();
}

bool NJOYProjectJsonIO::saveProject(SaveFormat saveFormat)
{
    QFile saveFile(saveFormat == jsonFormat
                   ? QStringLiteral("NJOYInput.json")
                   : QStringLiteral("NJOYInput.dat"));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open file.");
        return false;
    }

    QJsonObject obj;
    write(obj);
    saveFile.write(saveFormat == jsonFormat
                   ? QJsonDocument(obj).toJson()
                   : QCborValue::fromJsonValue(obj).toCbor());

    return true;
}

bool NJOYProjectJsonIO::loadProject(SaveFormat saveFormat)
{
    QFile loadFile(saveFormat == jsonFormat
                   ? QStringLiteral("NJOYInput.json")
                   : QStringLiteral("NJOYInput.dat"));

    if (!loadFile.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(saveFormat == jsonFormat
                          ? QJsonDocument::fromJson(saveData)
                          : QJsonDocument(QCborValue::fromCbor(saveData).toMap().toJsonObject()));

    read(loadDoc.object());

    QTextStream(stdout) << "Loaded save "
                        << " using "
                        << (saveFormat != jsonFormat ? "CBOR" : "JSON") << "...\n";

    return true;
}

void NJOYProjectJsonIO::read(const QJsonObject &json)
{
    if (json.contains(ChoosingIsotopesKey))
    {
        choosingIsotopes = std::make_unique<NJOYChoosingIsotopes>(
                    loadChoosingIsotopes(json[ChoosingIsotopesKey].toObject()));
    }

    if (json.contains(GeneralParameterKey))
    {
        generalParameters = std::make_unique<NJOYGeneralParametersInput>(
                    loadGeneralParametersInput(json[GeneralParameterKey].toObject()));
    }

    if (json.contains(ChoosingModulesKey))
    {
        choosingModules = loadChoosingModules(json[ChoosingModulesKey].toObject());
    }


    //Put other modules here
}

void NJOYProjectJsonIO::write(QJsonObject &json) const
{
    if (choosingIsotopes != nullptr)
        json[ChoosingIsotopesKey] = saveChoosingIsotopes();

    if (generalParameters != nullptr)
        json[GeneralParameterKey] = saveGeneralParameters();

    if (choosingModules)
        json[ChoosingModulesKey] = saveChoosingModules();
}

QJsonArray NJOYProjectJsonIO::saveReconrInput() const
{
    QJsonArray array;

    const auto reconrVect = choosingModules->getReconr();

    for (const auto & value : reconrVect)
    {
        QJsonObject obj;

        obj[CommentKey]   = QString::fromStdString(value->comment);
        obj[MatNumberKey] = QString::fromStdString(value->matNumber);
        obj[PrecisionKey] = QString::number(value->precision);

        array.append(obj);
    }

    return array;
}

std::vector<std::unique_ptr<NJOYReconrInput> > NJOYProjectJsonIO::loadReconrInput(const QJsonArray &array)
{
    std::vector<std::unique_ptr<NJOYReconrInput>> reconr;

    std::for_each(array.begin(), array.end(), [&](const QJsonValue item)
    {
        if (item.isObject())
        {
            auto reconrPtr = std::make_unique<NJOYReconrInput>();
            reconrPtr->comment   = item[CommentKey].toString().toStdString();
            //don't use item[PrecisionKey].toDouble, it doesn't work
            reconrPtr->precision = item[PrecisionKey].toVariant().toDouble();

            reconr.push_back(std::move(reconrPtr));
        }

    });

    return std::move(reconr);
}

QJsonArray NJOYProjectJsonIO::saveUnresrInput() const
{
    QJsonArray array;

    const auto unresrVect = choosingModules->getUnresr();

    for (const auto & value : unresrVect)
    {
        QJsonObject obj;

        obj[SigmasZeroKey]   = QString::fromStdString(value->sigmaZero);
        obj[TemperaturesKey] = QString::fromStdString(value->temperatures);
        obj[PrintOptionKey]  = QString::number(value->printOption);

        array.append(obj);
    }

    return array;
}

std::vector<std::unique_ptr<NJOYUnresInput> > NJOYProjectJsonIO::loadUnresrInput(const QJsonArray &array)
{
    std::vector<std::unique_ptr<NJOYUnresInput>> unresr;

    std::for_each(array.begin(), array.end(), [&](const QJsonValue item)
    {
        if (item.isObject())
        {
            auto unresrPtr = std::make_unique<NJOYUnresInput>();
            unresrPtr->sigmaZero    =  item[SigmasZeroKey].toString().toStdString();
            unresrPtr->temperatures =  item[TemperaturesKey].toString().toStdString();
            unresrPtr->printOption  =  item[PrintOptionKey].toInt();

            unresr.push_back(std::move(unresrPtr));
        }

    });

    return std::move(unresr);
}

void NJOYProjectJsonIO::setChoosingModules(std::unique_ptr<NJOYChoosingModulesJsonIO> &&newChoosingModules)
{
    choosingModules = std::move(newChoosingModules);
}

std::unique_ptr<NJOYChoosingModulesJsonIO> &&NJOYProjectJsonIO::getChoosingModules()
{
    if(!choosingModules)
        choosingModules = std::make_unique<NJOYChoosingModulesJsonIO>();

    return std::move(choosingModules);
}

std::unique_ptr<NJOYChoosingIsotopes> &&NJOYProjectJsonIO::getChoosingIsotopes()
{
    return std::move(choosingIsotopes);
}

std::unique_ptr<NJOYGeneralParametersInput> &&NJOYProjectJsonIO::getGeneralParameters()
{
    if (nullptr == generalParameters)
        generalParameters = std::make_unique<NJOYGeneralParametersInput>();

    return std::move(generalParameters);
}

void NJOYProjectJsonIO::setModule(std::unique_ptr<NJOYChoosingIsotopes> &&newChoosingIsotopes)
{
    if (nullptr == choosingIsotopes)
        choosingIsotopes = std::make_unique<NJOYChoosingIsotopes>();

    choosingIsotopes = std::move(newChoosingIsotopes);
}

void NJOYProjectJsonIO::setModule(std::unique_ptr<NJOYGeneralParametersInput> &&newGeneralParameter)
{
    if (nullptr == generalParameters)
        generalParameters = std::make_unique<NJOYGeneralParametersInput>();

    generalParameters = std::move(newGeneralParameter);
}

void NJOYProjectJsonIO::setModule(std::unique_ptr<NJOYChoosingModulesJsonIO> &&newNJOYChoosingModulesJsonIO)
{
    choosingModules = std::move(newNJOYChoosingModulesJsonIO);
}

NJOYChoosingModulesJsonIO &NJOYProjectJsonIO::getChoosingModulesObj()
{
    if(!choosingModules)
        choosingModules = std::make_unique<NJOYChoosingModulesJsonIO>();

    return *choosingModules;
}


