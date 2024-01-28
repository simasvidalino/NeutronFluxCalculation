#include "NJOYEnterDataWizard.h"

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPainter>

#include "NJOYChoosingIsotopesWidget.h"
#include "NJOYChoosingModulesWidget.h"
#include "NJOYGeneralParametersWidget.h"
#include "NJOYInterfaceDefinitions.h"
#include "NJOYWrapper.h"

#include "NJOYProjectJsonIO.h"

NJOYEnterDataWizard::NJOYEnterDataWizard(QWidget *parent)
    : QWizard{parent}
{
    addPages();

    wizardConfig();

    setConnections();
}

NJOYEnterDataWizard::~NJOYEnterDataWizard()
{

}

void NJOYEnterDataWizard::addPages()
{
    auto status = false;

    status += this->addPage(createIntroductionPage());
    status += this->addPage(createGeneralParametersPage());
    status += this->addPage(createChoosingIsotopesPage());
    status += this->addPage(createModulesPage());
    status += this->addPage(createExecuteNjoyPage());

    qInfo() << (status ? "Add all QWizardPage " :  "Add all QWizardPage Failled");
}

void NJOYEnterDataWizard::choosingIsotopesIO()
{

}

QWizardPage *NJOYEnterDataWizard::createIntroductionPage()
{
    QWizardPage *page = new QWizardPage;

    page->setTitle(tr(introductionTitle));

    auto mPixmap = QPixmap(":/Icons/atomo.jpg");
    //mPixmap = mPixmap.scaledToWidth(pixmapWizardWidth);

    setOpacity(mPixmap, 0.5);

    //    page->setPixmap(QWizard::WatermarkPixmap, mPixmap);

    QLabel *label = new QLabel(tr(introductionText));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);

    page->setLayout(layout);

    return page;
}

QWizardPage *NJOYEnterDataWizard::createChoosingIsotopesPage()
{
    auto page = new NJOYChoosingIsotopesWidget(this);

    page->setTitle(tr(choosingIsotopesTitle));

    return page;
}

QWizardPage *NJOYEnterDataWizard::createGeneralParametersPage()
{
    //    QWizardPage *page = new QWizardPage;

    //auto mPixmap = QPixmap(":/Icons/atomo.jpg");
    //mPixmap = mPixmap.scaledToWidth(pixmapWizardWidth);

    //page->setPixmap(QWizard::WatermarkPixmap, mPixmap);

    auto page = new NJOYGeneralParametersWidget(this);

    auto io = NJOYProjectJsonIO::getInstance();

    io->loadProject(jsonFormat);
    page->load(io->getGeneralParameters());

    page->setTitle(tr(generalParametersTitle));

    page->setSubTitle(tr(choosingIsotopesTitle));

    return page;
}

QWizardPage *NJOYEnterDataWizard::createModulesPage()
{
    //auto mPixmap = QPixmap(":/Icons/atomo.jpg");
    // mPixmap = mPixmap.scaledToWidth(pixmapWizardWidth);

    // page->setPixmap(QWizard::WatermarkPixmap, mPixmap);

    auto page = new NJOYChoosingModulesWidget(this);

    page->setTitle(tr(choosingModulesText));

    return page;
}

QWizardPage *NJOYEnterDataWizard::createExecuteNjoyPage()
{
    QWizardPage *page = new QWizardPage;

    page->setTitle(tr(choosingIsotopesTitle));

    //auto mPixmap = QPixmap(":/Icons/atomo.jpg");
    //mPixmap = mPixmap.scaledToWidth(pixmapWizardWidth);

    //page->setPixmap(QWizard::WatermarkPixmap, mPixmap);

    QLabel *label = new QLabel(tr(introductionText));
    QPushButton *runNjoyButton = new QPushButton("Run NJOY");

    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(label );
    layout->addWidget(runNjoyButton );

    page->setLayout(layout);

    auto runNjoy = [&](){
        QString filePath = QFileDialog::getSaveFileName(nullptr, "Save", "", "Text File (*.txt)");

        QStringList arguments;
        arguments << "-i" << "/home/andreiasimas/Área de trabalho/NJOY21/bin/inputModer" << "-o" << filePath ;

        if (!filePath.isEmpty()) {
            NJOYWrapper wrapper(this);
            wrapper.runNjoy("/home/andreiasimas/Área de trabalho/NJOY21/bin/njoy21", arguments); //home/andreiasimas/Documentos/TCCNJOY/UserInterface/build
        } else
        {
            qDebug() << "Seleção de arquivo cancelada pelo usuário";
        }

    };

    connect(runNjoyButton, &QPushButton::clicked, this, runNjoy);

    return page;
}

void NJOYEnterDataWizard::setConnections()
{
    connect(this, &QWizard::helpRequested, this, &NJOYEnterDataWizard::showHelp);

    connect(this, &QWizard::currentIdChanged, this, &NJOYEnterDataWizard::processDataBeforeEnteringNextPage);
}

void NJOYEnterDataWizard::setOpacity(QPixmap &input, double opacity)
{
    QImage image(input.size(), QImage::Format_ARGB32_Premultiplied); //Image with given size and format.
    image.fill(Qt::transparent); //fills with transparent

    QPainter p(&image);
    p.setOpacity(opacity); // set opacity from 0.0 to 1.0, where 0.0 is fully transparent and 1.0 is fully opaque.
    p.drawPixmap(0, 0, input); // given pixmap into the paint device.
    p.end();

    input = QPixmap::fromImage(image);
}

void NJOYEnterDataWizard::showHelp()
{
    QString message;

    switch (currentId())
    {
    case PageIntro:
        message = tr("The decision you make here will affect which page you "
                     "get to see next.");
        break;
    default:
        message = tr("This help is likely not to be of any help.");
    }

    QMessageBox::information(this, tr("License Wizard Help"), message);
}

void NJOYEnterDataWizard::wizardConfig()
{
    this->setOption(QWizard::HaveHelpButton, true);
    this->setOption(QWizard::NoCancelButton, true);

    this->setWizardStyle(QWizard::ClassicStyle);
}

void NJOYEnterDataWizard::processDataBeforeEnteringNextPage(const int pageNumber)
{
    qInfo()<<pageNumber;
}

template<typename wizardPage>
void NJOYEnterDataWizard::savePage(wizardPage *page)
{
    //Save and load
    auto io = NJOYProjectJsonIO::getInstance();
    connect(page, &QWizardPage::completeChanged, this, [&]()
    {
        if ((page->isComplete() == true)
                && (this->currentPage() == page))
        {
            qInfo()<<"Save";
            io->setModule(page->save());
            io->saveProject(jsonFormat);}
    });
}
