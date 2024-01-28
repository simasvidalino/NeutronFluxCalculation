#pragma once

#include <QWizardPage>

class NJOYChoosingModulesJsonIO;


namespace Ui {
class NJOYChoosingModulesWidget;
}

class NJOYChoosingModulesWidget : public QWizardPage
{
    Q_OBJECT

public:
    explicit NJOYChoosingModulesWidget(QWidget *parent = nullptr);
    ~NJOYChoosingModulesWidget();

    std::unique_ptr<NJOYChoosingModulesJsonIO> save() const ;

    void load(std::unique_ptr<NJOYChoosingModulesJsonIO> choosingModules);

private:
    Ui::NJOYChoosingModulesWidget *ui;

    enum NJOYMOdules
    {
        Acer,
        Broadr,
        Ccccr,
        Covr,
        Dtfr,
        Errorr,
        Gaminar,
        Gaspr,
        Groupr,
        Heatr,
        Leapr,
        Matxsr,
        Moder,
        Mixr,
        Plotr,
        Powr,
        Purr,
        Reconr,
        Resxsr,
        Thermr,
        Unresr,
        Viewr,
        Wimsr,

        NJOYMOdulesQtt
    };

    void createFile(const QString sourceFile, const QString targetFile);

    bool findMatNumberInTape(QString isotope, QString endfFile);

    void openBroadrEnterData();
\
    void openGrouprEnterData();

    void openModerEnterData();

    void openReconrEnterData();

    void openUnresEnterData();

    void setButtonGroup();

    void setConnections();

    void saveInformationFromOtherModule();

    QStringList sortModerTapes();

    template <typename T>
    std::vector<std::unique_ptr<T>>  fillWithDefault(T classObj, int numberOfData);

    QString reconrText;

    QString matNumbers;
    QStringList matNumberList ;
    QStringList fileNames;
    QString reactions;
    QString temperatures;
    std::unique_ptr<NJOYChoosingModulesJsonIO> choosingModules;

    // QWizardPage interface
public:
    virtual void initializePage() override;
    virtual bool validatePage() override;
    virtual bool isComplete() const override;
};

