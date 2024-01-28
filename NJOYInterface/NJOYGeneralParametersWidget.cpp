#include "NJOYGeneralParametersWidget.h"
#include "NJOYChoosingModulesJsonIO.h"
#include "ui_NJOYGeneralParametersWidget.h"

#include "NJOYChoosingModulesJsonIO.h"
#include "NJOYProjectJsonIO.h"

#include "NJOYInputTableWidget.h"
#include "NJOYTooltipsHeader.h"
#include <QTableWidget>


NJOYGeneralParametersWidget::NJOYGeneralParametersWidget(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::NJOYGeneralParametersWidget)
{
    ui->setupUi(this);

    setConnections();
    setTooltip();
}

NJOYGeneralParametersWidget::~NJOYGeneralParametersWidget()
{
    delete ui;
}

std::unique_ptr<NJOYGeneralParametersInput> NJOYGeneralParametersWidget::save() const
{
    auto general = std::make_unique<NJOYGeneralParametersInput>();

    general->matNumber    = ui->lineEditTarget->text().toStdString();
    general->reactions    = ui->lineEditReaction->text().toStdString();
    general->temperatures = ui->lineEditTemperature->text().toStdString();

    return general;
}

void NJOYGeneralParametersWidget::load(std::unique_ptr<NJOYGeneralParametersInput> general)
{
    if (nullptr == general)
        general = std::make_unique<NJOYGeneralParametersInput>();

    ui->lineEditReaction->setText(QString::fromStdString(general->reactions));
    ui->lineEditTarget->setText(QString::fromStdString(general->matNumber ));
    ui->lineEditTemperature->setText(QString::fromStdString(general->temperatures));
}

void NJOYGeneralParametersWidget::openInputTemperatures()
{
    NJOYInputTableWidget dlg(this);

    dlg.setColumnCount(1);
    dlg.setTableTitle("Temperature");
    dlg.setDataRange(1, 100);
    dlg.setHorizontalHeader("Kelvin", 0);

    if (false == dlg.exec())
        return;

    temperature = dlg.getColumn(0);
    ui->lineEditTemperature->setText(temperature);
}

void NJOYGeneralParametersWidget::openInputReactions()
{
    NJOYInputTableWidget dlg(this);
    dlg.setColumnCount(3);
    dlg.setTableTitle("NSUB = 10 (N) Incident-Neutron Data");
    dlg.hasItemCountity(false);

    QStringList mtNumberList;
    QStringList reactionList;
    QStringList reactionNameList;

    mtNumberList << "1" << "2" << "3" << "4" << "5" << "10" << "11" << "16" << "17" << "18" << "19"
                 << "20" << "21" << "22" << "23" << "24" << "25" << "27" << "28" << "29" << "32"
                 << "33" << "34" << "37" << "38" << "41" << "42" << "44" << "45" << "51" << "89"
                 << "90" << "91" << "101" << "102" << "103" << "104" << "105" << "106" << "107"
                 << "108" << "111" << "112" << "113" << "115" << "116" << "117" << "151" << "201"
                 << "202" << "203" << "204" << "205" << "206" << "207" << "208" << "209" << "210"
                 << "452" << "454" << "455" << "456" << "458" << "459" << "460" << "600" << "600" << "601"
                 << "649" << "650" << "650" << "651" << "699" << "700" <<  "700" << "701" << "749" << "750" << "751"
                 << "799" << "800" << "800" << "801" << "849" << "875" << "875" <<"876" << "876"
                 << "889" << "889" <<"890"<<"890";

    reactionList << "N,TOT" << "N,EL" << "N,NON" << "N,INL" << "N,X" << "N,TOT" << "N,2N+D" << "N,2N"
                 << "N,3N" << "N,F" << "N,F'" << "N,N+F" << "N,2N+F" << "N,N+A" << "N,N+3A" << "N,2N+A"
                 << "N,3N+A" << "N,ABS" << "N,N+P" << "N,N+2A" << "N,N+D" << "N,N+T" << "N,N+HE3"
                 << "N,4N" << "N,3N+F" << "N,2N+P" << "N,3N+P" << "N,N+2P" << "N,N+P+A" << "N,N'"
                 << "N,N'" << "N,N'" << "N,N'" << "N,DIS" << "N,G" << "N,P" << "N,D" << "N,T" << "N,HE3"
                 << "N,A" << "N,2A" << "N,2P" << "N,P+A" << "N,T+2A" << "N,P+D" << "N,P+T" << "N,D+A"
                 << "N,RES" << "N,XN"
                 << "N,XG" << "N,XP" << "N,XD" << "N,XT" << "N,XHE3" << "N,XA" << "N,XPi_pos"
                 << "N,XPi_0" << "N,XPi_neg" << "N,nu_tot" << "N,ind_FY" << "N,nu_d" << "N,nu_p"
                 << "N,rel_fis" << "FY_cum" << "N,g_bdf"
                 << "N,P'" << "N,P" << "N,P'" << "N,P'"
                 << "N,D'" << "N,D" << "N,D'"<< "N,D'"
                 << "N,T'" << "N,T" << "N,T'" << "N,T'"
                 << "N,HE3'" << "N,HE3'" << "N,HE3'"
                 << "N,A'" << "N,A" << "N,A'" << "N,A'"
                 << "N,2N'" << "N,2N" << "N,2N" << "N,2N'" << "N,2N'" << "N,2N" << "N,2N" << "N,2N'";

    reactionNameList<< "Neutron total cross sections"
                    << "Elastic scattering cross section for incident particles"
                    << "Nonelastic neutron cross section."
                    << "Production of one neutron in the exit channel."
                    << "Sum of all reactions not given explicitly in another MT number."
                    << "Total continuum reaction."
                    << "Production of two neutrons and a deuteron, plus a residual."
                    << "Production of two neutrons and a residual."
                    << "Production of three neutrons and a residual."
                    << "Total fission."
                    << "Total fission (for neutrons only)."
                    << "Second-chance fission."
                    << "Third-chance fission."
                    << "Production of a neutron and an alpha particle, plus a residual."
                    << "Production of a neutron and three alpha particles, plus a residual."
                    << "Production of two neutrons and an alpha particle, plus a residual."
                    << "Production of three neutrons and an alpha particle, plus a residual."
                    << "Absorption; sum of MT=18 and MT=102 through MT=117"
                    << "Production of a neutron and a proton, plus a residual."
                    << "Production of a neutron and two alpha particles, plus a residual."
                    << "Production of a neutron and a deuteron, plus a residual."
                    << "Production of a neutron and a triton, plus a residual."
                    << "Production of a neutron and a 3He particle, plus a residual."
                    << "Production of 4 neutrons, plus a residual."
                    << "Fourth-chance fission cross section."
                    << "Production of 2 neutrons and a proton, plus a residual."
                    << "Production of 3 neutrons and a proton, plus a residual."
                    << "Production of a neutron and 2 protons, plus a residual."
                    << "Production of a neutron, a proton, and an alpha particle, plus a residual."
                    << "Production of a neutron, with residual in the 1st excited state."
                    << "Production of a neutron, with residual in the 39-excited state."
                    << "Production of a neutron, with residual in the 40-excited state."
                    << "Production of a neutron in the continuum not included in the discrete representation."
                    << "Neutron disappearance; equal to sum of MT=102-117."
                    << "Radiative capture."
                    << "Production of a proton, plus a residual. Sum of MT=600-649, if they are present."
                    << "Production of a deuteron, plus a residual. Sum of MT=650-699, if they are present."
                    << "Production of a triton, plus a residual. Sum of MT=700-749, if they are present."
                    << "Production of a 3He particle plus a residual. Sum of MT=750-799, if they are present."
                    << "Production of an alpha particle, plus a residual. Sum of MT=800-849, if they are present."
                    << "Production of 2 alpha particles, plus a residual."
                    << "Production of 2 protons, plus a residual."
                    << "Production a proton and an alpha particle, plus a residual."
                    << "Production of a triton and 2 alpha particles, plus a residual."
                    << "Production of a deuteron and 2 alpha particles, plus a residual."
                    << "Production of proton and a triton, plus a residual."
                    << "Production of deuteron and an alpha particle, plus a residual."
                    << "Resonance parameters that can be used to calculate cross sections at different temperatures."
                    << "Total neutron production."
                    << "Total gamma production."
                    << "Total proton production."
                    << "Total deuteron production."
                    << "Total triton production."
                    << "Total He-3 production."
                    << "Total alpha particle production."
                    << "Total Pi+ production."
                    << "Total Pi0 production."
                    << "Total Pi- production."
                    << "Average total (prompt plus delayed) number of neutrons released per fission event."
                    << "Independent fission product yield data."
                    << "Average number of delayed neutrons released per fission event."
                    << "Average number of prompt neutrons released per fission event."
                    << "Energy release in fission for incident neutrons."
                    << "Cumulative fission product yield data."
                    << "Beta-Delayed Fission Gamma Data."
                    << "Production of a proton leaving the residual nucleus in the ground state."
                    << "Production of a proton leaving the residual nucleus in the ground state."
                    << "Production of a proton, with residual in the 1st excited state."
                    << "Production of a proton in the continuum."
                    << "Production of a deuteron leaving the residual nucleus in the ground state."
                    << "Production of a deuteron leaving the residual nucleus in the ground state."
                    << "Production of a deuteron, with residual in the 1st excited state."
                    << "Production of a deuteron in the continuum."
                    << "Production of a triton leaving the residual nucleus in the ground state."
                    << "Production of a triton leaving the residual nucleus in the ground state."
                    << "Production of a triton, with residual in the 1st excited state."
                    << "Production of a triton in the continuum."
                    << "Production of a He-3 leaving the residual nucleus in the ground state."
                    << "Production of a He-3, with residual in the 1st excited state."
                    << "Production of a He-3 in the continuum."
                    << "Production of an alpha leaving the residual nucleus in the ground state."
                    << "Production of an alpha leaving the residual nucleus in the ground state."
                    << "Production of an alpha, with residual in the 1st excited state."
                    << "Production of an alpha in the continuum."
                    << "Production of 2 neutrons leaving the residual nucleus in the ground state."
                    << "Production of 2 neutrons leaving the residual nucleus in the ground state."
                    << "Production of 2 neutrons, with residual in the 1st excited state."
                    << "Production of 2 neutrons, with residual in the 1-st excited state."
                    << "Production of 2 neutrons, with residual in the 14-excited state."
                    << "Production of 2 neutrons, with residual in the 14-excited state."
                    << "Production of 2 neutrons, with residual in the 15-excited state."
                    << "Production of 2 neutrons, with residual in the 15-excited state.";

    qInfo()<<"TIRA A TEIMA "<<mtNumberList.size()<<" "<<reactionList.size()<<" "<<reactionNameList.size();

    dlg.setRowCount(reactionList.count());

    dlg.setColumn(mtNumberList, 0);
    dlg.setColumn(reactionList, 1);
    dlg.setColumn(reactionNameList, 2, 300);

    dlg.setHorizontalHeader("MT", 0);
    dlg.setHorizontalHeader("Reaction", 1);
    dlg.setHorizontalHeader("Description", 2);

    dlg.setGeometry(100, 0, 600, 400);

    if (false == dlg.exec())
        return;

    reaction = dlg.getSelectedText(1);
    ui->lineEditReaction->setText(reaction);
}

std::vector<int> NJOYGeneralParametersWidget::separateMatNumber()
{
    std::vector <int> matVector;
    bool ok = false;

    const auto reationList = ui->lineEditTarget->text().split(";").toVector();

    std::for_each(reationList.begin(), reationList.end(), [&](QString data)
    {
        auto reactionNumber = data.toInt(&ok);

        if (ok)
            matVector.push_back(reactionNumber);
        else
            qInfo()<<"Error separate reaction";
    });

    return matVector;
}

std::vector<int> NJOYGeneralParametersWidget::separateReaction()
{
    std::vector <int> reactionVector;
    bool ok = false;

    const auto reationList = ui->lineEditReaction->text().split(";").toVector();

    std::for_each(reationList.begin(), reationList.end(), [&](QString data)
    {
        auto reactionNumber = data.toInt(&ok);

        if (ok)
            reactionVector.push_back(reactionNumber);
        else
            qInfo()<<"Error separate reaction";
    });

    return reactionVector;
}

std::vector<double> NJOYGeneralParametersWidget::separateTemperatura()
{
    std::vector <double> reactionVector;
    bool ok = false;

    const auto reationList = ui->lineEditReaction->text().split(";").toVector();

    std::for_each(reationList.begin(), reationList.end(), [&](QString data)
    {
        auto reactionNumber = data.toInt(&ok);

        if (ok)
            reactionVector.push_back(reactionNumber);
        else
            qInfo()<<"Error separate temperature";
    });

    return reactionVector;
}

void NJOYGeneralParametersWidget::setConnections()
{
    connect(ui->commandLinkButtonTemperature, &QCommandLinkButton::clicked, this,
            &NJOYGeneralParametersWidget::openInputTemperatures);
    connect(ui->commandLinkButtonReaction, &QCommandLinkButton::clicked, this,
            &NJOYGeneralParametersWidget::openInputReactions);

    connect(ui->lineEditReaction, &QLineEdit::editingFinished, this,
            [this](){emit completeChanged();});
    connect(ui->lineEditTarget, &QLineEdit::editingFinished, this,
            [this](){emit completeChanged();});
    connect(ui->lineEditTemperature, &QLineEdit::editingFinished, this,
            [this](){emit completeChanged();});

}

void NJOYGeneralParametersWidget::setTooltip()
{
    ui->lineEditReaction->setToolTipDuration(1000);
    ui->lineEditReaction->setToolTip(reactionToolTip);
}

void NJOYGeneralParametersWidget::initializePage()
{
    auto io = NJOYProjectJsonIO::getInstance();

    io->loadProject(jsonFormat);
    load(io->getGeneralParameters());
}

bool NJOYGeneralParametersWidget::validatePage()
{
    auto io = NJOYProjectJsonIO::getInstance();

    //delete if we have any information from other modules

    io->deleteAllModules();
    io->setModule(save());
    io->saveProject(jsonFormat);

    return true;
}

bool NJOYGeneralParametersWidget::isComplete() const
{
    bool check = (!ui->lineEditReaction->text().isEmpty())
            && (!ui->lineEditTarget->text().isEmpty())
            && (!ui->lineEditTemperature->text().isEmpty());

    qInfo()<<"NJOYGeneralParametersWidget::isComplete() "<<check;

    return check;
}
