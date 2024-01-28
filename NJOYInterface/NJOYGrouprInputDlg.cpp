#include "NJOYGrouprInputDlg.h"
#include "ui_NJOYGrouprInputDlg.h"

#include <QInputDialog>

#include "NJOYInputTableWidget.h"

#include "NJOYChoosingModulesJsonIO.h"

#include "NJOYGroupInputweightFunctionOptions.h"
#include "NJOYProjectJsonIO.h"

NJOYGrouprInputDlg::NJOYGrouprInputDlg(QWidget *parent) :
    NJOYModulesBase(parent),
    ui(new Ui::NJOYGrouprInputDlg),
    numberOfTemperature(1)
{
    ui->setupUi(this);

    //See Test Problem 17

    setToolTips();

    configWidgets();

    setConnections();
}

NJOYGrouprInputDlg::~NJOYGrouprInputDlg()
{
    delete ui;
}

std::unique_ptr<NJOYGrouprInput> NJOYGrouprInputDlg::save()
{
    auto groupr = std::make_unique<NJOYGrouprInput>();

    groupr->title = ui->lineEditTitle->text().toStdString();

    groupr->MTDOption = currentRowMFD;

    //The last value from a combobox is not a option, you have to type one number.
    currentRIWT = ui->comboBoxIWT->currentIndex();
    if (RIWTN.has_value())
        groupr->weightFunctionOption = currentRIWT; //typed number

    groupr->longPrintOption      = ui->comboBoxIPRINT->currentIndex();
    groupr->smoothOption         = ui->comboBoxIsmooth->currentIndex();
    groupr->numberOfGammaGroup   = ui->comboBoxIGG->currentIndex();
    groupr->neutronGroupStructure= ui->comboBoxIGN->currentIndex();

    //tbd
    //groupr->fileAndSectionTobeProcessed = ui->
    if (numberOfGroupBreaks.size() > 0)
    {
        bool ok = false;

        groupr->numberOfNeutronGroups = numberOfGroupBreaks.count();

        std::for_each(numberOfGroupBreaks.begin(), numberOfGroupBreaks.end(), [&](QString str){
            groupr->energyNeutronGroups->push_back(str.toInt(&ok));
        });

        if (!ok)
            qInfo()<<"NJOYGrouprInputDlg::save: unit convertion error at string";
    }

    //@todo one function for feed vector<int>
    if (numberOfGammaBreaks.size() > 0)
    {
        bool ok = false;

        groupr->numberOfGammaGroup = numberOfGammaBreaks.count();

        std::for_each(numberOfGammaBreaks.begin(), numberOfGammaBreaks.end(), [&](QString str){
            groupr->energyGammaGroups->push_back(str.toInt(&ok));
        });

        if (!ok)
            qInfo()<<"NJOYGrouprInputDlg::save: unit convertion error at string";
    }

    return groupr;
}

void NJOYGrouprInputDlg::load(std::unique_ptr<NJOYGrouprInput> groupr)
{
    if (nullptr == groupr)
        groupr = std::make_unique<NJOYGrouprInput>();

    auto weight = std::make_unique<NJOYGrouprWeightFunctions>(groupr->weightFunctions);

    ui->lineEditTitle->setText(groupr->title.c_str());

    currentRowMFD = groupr->MTDOption;

    if (groupr->weightFunctionOptionN.has_value())
        currentRIWT = groupr->weightFunctionOption;

    ui->comboBoxIPRINT->setCurrentIndex(ui->comboBoxIPRINT->currentIndex());
    ui->comboBoxIsmooth->setCurrentIndex(ui->comboBoxIsmooth->currentIndex());
    ui->comboBoxIGG->setCurrentIndex(ui->comboBoxIGG->currentIndex());
    ui->comboBoxIGN->setCurrentIndex(ui->comboBoxIGN->currentIndex());

    //tbd
    //groupr->fileAndSectionTobeProcessed = ui->
    if (groupr->numberOfNeutronGroups.has_value())
    {
        auto vect = groupr->energyNeutronGroups.value();
        numberOfGroupBreaks.clear();
        std::for_each(vect.begin(), vect.end(), [&](long double value){
            numberOfGroupBreaks.push_back(QString::number(value, 'g', 10));
        });
    }

    if (groupr->energyGammaGroups.has_value())
    {
        auto vect = groupr->energyGammaGroups.value();
        numberOfGammaBreaks.clear();
        std::for_each(vect.begin(), vect.end(), [&](long double value){
            numberOfGammaBreaks.push_back(QString::number(value, 'g', 10));
        });
    }

    //@todo one function for feed vector<int>
    if (numberOfGammaBreaks.size() > 0)
    {
        bool ok = false;

        groupr->numberOfGammaGroup = numberOfGammaBreaks.count();

        std::for_each(numberOfGammaBreaks.begin(), numberOfGammaBreaks.end(), [&](QString str){
            groupr->energyGammaGroups->push_back(str.toInt(&ok));
        });

        if (!ok)
            qInfo()<<"NJOYGrouprInputDlg::save: unit convertion error at string";
    }
}

void NJOYGrouprInputDlg::onOpenGrouprWeightFunctions()
{
    NJOYGroupInputweightFunctionOptions dlg(this);

    if (!dlg.exec())
        return;

    dlg.save();
}

void NJOYGrouprInputDlg::onInputTemperatures()
{
    NJOYInputTableWidget dlg(this);

    dlg.setColumnCount(1);
    dlg.setTableTitle("Temperature");
    dlg.setDataRange(1, 100);
    dlg.setHorizontalHeader("Kelvin", 0);

    if (false == dlg.exec())
        return;

    temperature = dlg.getColumn(0);
}

void NJOYGrouprInputDlg::onInputSigmaZero()
{
    NJOYInputTableWidget dlg(this);

    dlg.setColumnCount(1);
    dlg.setTableTitle("Input Sigmas zero");
    dlg.setDataRange(1, 10);
    dlg.setHorizontalHeader("Kelvin", 0);

    if (false == dlg.exec())
        return;

    temperature = dlg.getColumn(0);

}

void NJOYGrouprInputDlg::populateComboboxIGN()
{
    QList <QString> groupTypeList;
    groupTypeList << "arbitrary structure (read in)"
                  << "csewg 239-group structure"
                  << "lanl 30-group structure"
                  << "anl 27-group structure"
                  << "rrd 50-group structure"
                  << "am-i 68-group structure"
                  << "gam-ii 100-group struct"
                  << "laser-thermos 35-group structure"
                  << "epri-cpm 69-group structure"
                  << "lanl 187-group structure"
                  << "lanl 70-group structure"
                  << "sand-ii 620-group structure"
                  << "lanl 80-group structure"
                  << "eurlib 100-group structure"
                  << "sand-iia 640-group structure"
                  << "vitamin-e 174-group structure"
                  << "vitamin-j 175-group structure"
                  << "xmas nea-lanl"
                  << "ecco 33-group structure"
                  << "ecco 1968-group structure"
                  << "tripoli 315-group structure"
                  << "xmas lwpc 172-group structure"
                  << "vit-j lwpc 175-group structure"
                  << "shem cea 281-group structure"
                  << "shem epm 295-group structure"
                  << "shem cea/epm 361-group structure"
                  << "shem epm 315-group structure"
                  << "rahab aecl 89-group structure"
                  << "ccfe 660-group structure (30 MeV)"
                  << "ukaea 1025-group structure (30 MeV)"
                  << "ukaea 1067-group structure (200 MeV)"
                  << "ukaea 1102-group structure (1 GeV)"
                  << "ukaea 142-group structure (200 MeV)"
                  << "(200 MeV)"
                  << "lanl 618-group structure";

    ui->comboBoxIGN->addItems(groupTypeList);
}

void NJOYGrouprInputDlg::populateComboboxIGG()
{
    QList <QString> groupTypeList;
    groupTypeList << "none"
                  << "arbitrary structure (read in)"
                  << "csewg 94-group structure"
                  << "lanl 12-group structure"
                  << "steiner 21-group gamma-ray structure"
                  << "straker 22-group structure"
                  << "lanl 48-group structure"
                  << "lanl 24-group structure"
                  << "vitamin-c 36-group structure"
                  << "vitamin-e 38-group structure"
                  << "vitamin-j 42-group structure";

    ui->comboBoxIGG->addItems(groupTypeList);
}

void NJOYGrouprInputDlg::populateComboboxIsSmooth()
{
    QList <QString> groupTypeList;
    groupTypeList << "on" << "off";

    ui->comboBoxIPRINT->addItems(groupTypeList);
}

void NJOYGrouprInputDlg::populateComboboxIPRINT()
{
    QList <QString> groupTypeList;
    groupTypeList << "maximum" << "minimum";

    ui->comboBoxIPRINT->addItems(groupTypeList);
}

void NJOYGrouprInputDlg::populateComboboxIWT()
{
    QList <QString> groupTypeList;
    groupTypeList << "read in resonance flux from ninwt"
                  << "read in smooth weight function"
                  << "constant"
                  << "1/e"
                  << "1/e + fission spectrum + thermal maxwellian"
                  << "epri-cell lwr"
                  << "(thermal) -- (1/e) -- (fission + fusion)"
                  << "thermal--1/e--fast reactor--fission + fusion"
                  << "same with t-dep thermal part"
                  << "claw weight function"
                  << "claw with t-dependent thermal part"
                  << "vitamin-e weight function (ornl-5505)"
                  << "vit-e with t-dep thermal part"
                  << "compute flux with weight n";

    ui->comboBoxIWT->addItems(groupTypeList);

//    QStandardItemModel *model = new QStandardItemModel(this);

//    for (const auto & item : groupTypeList)
//    {
//        QStandardItem *editableItem = new QStandardItem(item);
//        model->appendRow(editableItem);
//    }

//    // Add a editable item
//    QStandardItem *editableItem = new QStandardItem("compute flux with weight n");
//    editableItem->setEditable(true);
//    editableItem->setFlags(editableItem->flags() | Qt::ItemIsEditable);

//    model->appendRow(editableItem);

//    ui->comboBoxIWT->setModel(model);
}

void NJOYGrouprInputDlg::populateComboboxMFD()
{
    QList <QString> groupTypeList;
    ui->comboBoxIGG->addItems(groupTypeList);
}

void NJOYGrouprInputDlg::onInputTableEGN()
{
    NJOYInputTableWidget dlg(this);

    dlg.setTableTitle("eV");

    dlg.setColumn(numberOfGroupBreaks, First);
    dlg.setHorizontalHeader("Group breaks", First);

    if (!dlg.exec())
        return;

    numberOfGroupBreaks = dlg.getColumnList(First);
}

void NJOYGrouprInputDlg::onInputTableMFD()
{
    QList <QString> mfdList;
    QList <QString> valueList;

//@todo use it only for help context
//    mfdList << "cross section or yield vector"
//            << "fission chi by short-cut method"
//            << "neutron-neutron matrix (mf4/5)"
//            << "neutron-neutron matrix (mf6)"
//            << "photon prod. xsec (photon yields given, mf12)"
//            << "photon prod. xsec (photon xsecs given, mf13)"
//            << "neutron-gamma matrix (photon yields given)"
//            << "neutron-gamma matrix (photon xsecs given)"
//            << "neutron-gamma matrix (mf6)"
//            << "proton production matrix (mf6)"
//            << "deuteron production (mf6)"
//            << "triton production (mf6)"
//            << "he-3 production (mf6)"
//            << "alpha production (mf6)"
//            << "residual nucleus (a>4) production (mf6)"
//            << "proton production matrix (mf4)"
//            << "deuteron production (mf4)"
//            << "triton production (mf4)"
//            << "he-3 production (mf4)"
//            << "alpha production (mf4)"
//            << "residual nucleus (a>4) production (mf4)"
//            << "nuclide production for zzzaaam File 3"
//            << "nuclide production for zzzaaam File 6"
//            << "nuclide production for zzzaaam File 9"
//            << "nuclide production for zzzaaam File 10"
//            << "fission product production (mtd=18 only) File 10";

//    valueList << "3"  << "5"  << "6"  << "8"  << "12" << "13" << "16"
//              << "17" << "18" << "21" << "22" << "23" << "24" << "25"
//              << "26" << "31" << "32" << "33" << "34" << "35" << "36"
//              << "1zzzzaaam" << "2zzzaaam"  << "3zzzaaam" << "4zzzaaam"
//              << "40000000";

//    int index = 0;
//    for (QString &item:  mfdList)
//    {
//        item.prepend((valueList.at(index)) + QString(" "));

//        ++index;
//    }

    auto io = NJOYProjectJsonIO::getInstance();

    NJOYInputTableWidget dlg(this);

    dlg.setTableTitle("MTD");
    dlg.setExplanation(fileAndSectionToBeProcessedExplanation);
    dlg.setColumn(QString::fromStdString(io->getGeneralParameters()->reactions), First);
    dlg.setCurrentRow(currentRowMFD);

    if (!dlg.exec())
        return;

    currentRowMFD = dlg.getSelectedRow();
}

void NJOYGrouprInputDlg::onInputTableMTD()
{
    QList <QString> mtdList;
    QList <QString> meaningList;

    mtdList << "-n" << "221-250" << "257" << "258" << "259";
    meaningList << "process all mt numbers from the previous entry to n inclusive"
                << "reserved for thermal scattering"
                << "average energy"
                << "average lethargy"
                << "average inverse velocity (m/sec)";

    NJOYInputTableWidget dlg(this);

    dlg.setTableTitle("Section to be processed (MTD)");
    // dlg.setVerticalHeader(mtdList);
    dlg.setCurrentRow(currentRowMTD);

    if (!dlg.exec())
        return;

    currentRowMTD = dlg.getSelectedRow();
}

void NJOYGrouprInputDlg::configWidgets()
{
    populateComboboxIGN();

    populateComboboxIGG();

    populateComboboxIPRINT();

    populateComboboxIWT();

    setToolTips();

    ui->lineEditTitle->setMaxLength(84);
}

void NJOYGrouprInputDlg::setConnections()
{
    connect(ui->comboBoxIGN, &QComboBox::currentIndexChanged, this, [&](int index)
    {
        bool check = (index == 0);
        ui->pushButtonNGN->setVisible(check);
    });

    connect(ui->comboBoxIGG, &QComboBox::currentIndexChanged, this, [&](int index)
    {
        bool check = (index == 0);
        ui->pushButtonNGG->setVisible(check);
    });

    connect(ui->pushButtonNGN, &QPushButton::clicked, this,
            &NJOYGrouprInputDlg::onInputTableEGN);

    connect(ui->pushButtonMFD, &QPushButton::clicked, this,
            &NJOYGrouprInputDlg::onInputTableMFD);

    connect(ui->pushButtonMTD, &QPushButton::clicked, this,
            &NJOYGrouprInputDlg::onInputTableMTD);

    connect(ui->comboBoxIWT, &QComboBox::currentIndexChanged, this, [&](int index)
    {
        bool check = (index == 0 || index == ui->comboBoxIWT->count()-1);
        ui->pushButtonWeightFunctionsOptions->setVisible(check);

        if (index == ui->comboBoxIWT->count()-1)
        {
            bool ok;
            int value = QInputDialog::getInt(this, tr("QInputDialog::getInt()"),
                                         tr("Type the weight:"), 0, 0, 200, 1, &ok);
            if (ok)
                RIWTN = value;

        }
    });



}

void NJOYGrouprInputDlg::setToolTips()
{
    ui->comboBoxIsmooth->setToolTip(issSmoothToolTip);
}
