#include "NJOYChoosingIsotopesWidget.h"
#include "NJOYChoosingModulesJsonIO.h"
#include "NJOYInterfaceDefinitions.h"
#include "ui_NJOYChoosingIsotopesWidget.h"

#include <memory.h>

#include <QFileDialog>
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "NJOYProjectJsonIO.h"

NJOYChoosingIsotopesWidget::NJOYChoosingIsotopesWidget(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::NJOYChoosingIsotopesWidget)
{
    ui->setupUi(this);

    setLink();

    setButtonGroup();

    hideWidgets();

    setConnections();
}

NJOYChoosingIsotopesWidget::~NJOYChoosingIsotopesWidget()
{
    delete ui;
}

std::unique_ptr<NJOYChoosingIsotopes> NJOYChoosingIsotopesWidget::save()
{
    auto choosingIsotope = std::make_unique<NJOYChoosingIsotopes>();

    for(const auto &name : fileNames)
        choosingIsotope->fileNames.push_back(name.toStdString());

    return choosingIsotope;
}

void NJOYChoosingIsotopesWidget::load(std::unique_ptr<NJOYChoosingIsotopes> choosingIso)
{
    if (nullptr == choosingIso)
        choosingIso = std::make_unique<NJOYChoosingIsotopes>();

    ui->radioButtonYes->setChecked(true);
    setEndfFileOption(Yes);

    fileNames.clear();
    for (auto const & name : choosingIso->fileNames)
        fileNames.push_back(QString::fromStdString(name));

    if (!fileNames.isEmpty())
    {
        setFileNameInLabel();
    }
}

void NJOYChoosingIsotopesWidget::openFile()
{
    fileNames = QFileDialog::getOpenFileNames(this, tr("Open File"), "/home",
                                              tr("Text (*.txt *.dat)"));
    qInfo()<<fileNames;

    setFileNameInLabel();
}

void NJOYChoosingIsotopesWidget::setButtonGroup()
{
    ui->buttonGroup->setId(ui->radioButtonNo, No);
    ui->buttonGroup->setId(ui->radioButtonYes, Yes);
}

void NJOYChoosingIsotopesWidget::setLink()
{
    ui->labelLink->setText(iaeaPath);
    ui->labelLink->setToolTip("https://www-nds.iaea.org/exfor/endf.htm");
    ui->labelLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->labelLink->setOpenExternalLinks(true);
}

void NJOYChoosingIsotopesWidget::startFadeEffect(QWidget * w, int fadeDuration)
{
    auto eff = std::make_unique<QGraphicsOpacityEffect>(this);

    w->setGraphicsEffect(eff.get());

    auto animation = new QPropertyAnimation(eff.release(),"opacity");

    animation->setDuration(fadeDuration);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->setEasingCurve(QEasingCurve::InBack);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void NJOYChoosingIsotopesWidget::initializePage()
{
    auto io = NJOYProjectJsonIO::getInstance();

    io->loadProject(jsonFormat);
    load(io->getChoosingIsotopes());
}

bool NJOYChoosingIsotopesWidget::validatePage()
{
    auto io = NJOYProjectJsonIO::getInstance();

    io->setModule(save());
    io->saveProject(jsonFormat);

    return true;
}

bool NJOYChoosingIsotopesWidget::isComplete() const
{
    bool isComplete = !fileNames.isEmpty();

    qInfo()<<"Is complete";

    setToolTipForNextButton();

    return isComplete;
}

void NJOYChoosingIsotopesWidget::setConnections()
{
    connect(ui->toolButtonOpenFile, &QToolButton::clicked, this,
            &NJOYChoosingIsotopesWidget::openFile);

    connect(ui->buttonGroup, &QButtonGroup::idClicked, this,
            &NJOYChoosingIsotopesWidget::setEndfFileOption);

    connect(ui->labelLink, &QLabel::linkActivated, this,
            [](const QString& link){qInfo()<<"clicou "<<link;});
}

void NJOYChoosingIsotopesWidget::setFileNameInLabel()
{
    QString qFileNames;

    for (const auto &file : fileNames)
    {
        auto mFile = file.split("/").back();
        mFile.push_front("->");

        qFileNames += mFile;
        qFileNames.push_back("\n");
    }

    ui->labelFileName->setText(qFileNames);
    changeOrNotTapesSaved();

    emit completeChanged();
}

void NJOYChoosingIsotopesWidget::setToolTipForNextButton() const
{
    bool isComplete = !fileNames.isEmpty();

    QAbstractButton* nextButton = wizard()->button(QWizard::NextButton);

    if (nullptr != nextButton)
    {
        if (isComplete)
        {
            nextButton->setToolTip(choosingIsotopesNextButtoTooltipOk);
        }
        else
        {
            nextButton->setToolTip(choosingIsotopesNextButtoTooltip);
        }
    }
}

void NJOYChoosingIsotopesWidget::hideWidgets()
{
    ui->labelSiteInformation->hide();
    ui->labelLink->hide();

    ui->labelChooseENDFFile->hide();
    ui->toolButtonOpenFile->hide();
}

QStringList NJOYChoosingIsotopesWidget::getFileNames() const
{
    return fileNames;
}

void NJOYChoosingIsotopesWidget::setEndfFileOption(int option)
{
    hideWidgets();
    int normalFade = 350;
    int longFade = 500;

    switch (option)
    {
    case Yes:
        ui->labelChooseENDFFile->show();
        startFadeEffect(ui->labelChooseENDFFile, normalFade);

        ui->toolButtonOpenFile->show();
        startFadeEffect(ui->toolButtonOpenFile, normalFade);
        break;
    case No:
        ui->labelChooseENDFFile->show();
        startFadeEffect(ui->labelChooseENDFFile, longFade);

        ui->toolButtonOpenFile->show();
        startFadeEffect(ui->toolButtonOpenFile, longFade);

        ui->labelSiteInformation->show();
        startFadeEffect(ui->labelSiteInformation, normalFade);

        ui->labelLink->show();
        startFadeEffect(ui->labelLink, normalFade);
        break;
    default:
        break;
    }
}

void NJOYChoosingIsotopesWidget::changeOrNotTapesSaved()
{
    hideWidgets();
    int normalFade = 350;
    int longFade = 800;

    ui->labelInformation->setText("Tapes choosen:");
    startFadeEffect(ui->labelInformation, longFade);
    ui->labelInformation->setText("You have already had tape(s) choosen before, if you want to change it, lets start:\nDo you have ENDF files?");
}

