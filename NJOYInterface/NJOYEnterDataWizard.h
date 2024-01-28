#pragma once

#include <QWizard>

class NJOYEnterDataWizard : public QWizard
{
    Q_OBJECT
public:
    explicit NJOYEnterDataWizard(QWidget *parent = nullptr);

    ~NJOYEnterDataWizard();

private:

    enum PageEnum
    {
        PageIntro,
        PageChoosingIsotopes,
        PageChoosingModules
    };

    void addPages();

    void choosingIsotopesIO();

    QWizardPage *createChoosingIsotopesPage();
    QWizardPage *createGeneralParametersPage();
    QWizardPage *createIntroductionPage();
    QWizardPage *createModulesPage();
    QWizardPage *createExecuteNjoyPage();

    int getNumberPage();

    void openFile();

    template<typename wizardPage>
    void savePage(wizardPage *page);

    void setConnections();

    void setOpacity(QPixmap &input, double opacity);

    void setStatus();

    void showHelp();

    void wizardConfig();

    void processDataBeforeEnteringNextPage(const int pageNumber);
};

