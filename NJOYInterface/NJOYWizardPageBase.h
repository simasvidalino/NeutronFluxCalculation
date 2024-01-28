#pragma once

#include <QWizardPage>

struct NJOYBase;

class NJOYWizardPageBase : public QWizardPage
{
public:
    NJOYWizardPageBase(QWidget *parent = nullptr);

    virtual std::unique_ptr< NJOYBase> save() = 0;
    virtual void load(std::unique_ptr<NJOYBase> choosingIso) = 0;
};

