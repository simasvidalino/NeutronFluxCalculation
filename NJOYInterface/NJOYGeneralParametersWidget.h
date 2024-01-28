#pragma once

#include <QWizardPage>

struct NJOYGeneralParametersInput;

namespace Ui {
class NJOYGeneralParametersWidget;
}

class NJOYGeneralParametersWidget : public QWizardPage
{
    Q_OBJECT

public:
    explicit NJOYGeneralParametersWidget(QWidget *parent = nullptr);
    ~NJOYGeneralParametersWidget();

    std::unique_ptr<NJOYGeneralParametersInput> save() const;
    void load(std::unique_ptr<NJOYGeneralParametersInput> general);

private:
    Ui::NJOYGeneralParametersWidget *ui;

    void openInputTemperatures();
    void openInputReactions();

    std::vector<int> separateMatNumber();
    std::vector<int> separateReaction();
    std::vector<double> separateTemperatura();

    void setConnections();
    void setTooltip();

    QString reaction;
    QString temperature;

    // QWizardPage interface
public:
    virtual void initializePage() override;
    virtual bool validatePage() override;
    virtual bool isComplete() const override;
};

