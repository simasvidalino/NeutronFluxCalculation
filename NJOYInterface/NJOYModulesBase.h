#pragma once

#include <QDialog>

class NJOYModulesBase : public QDialog
{
public:
    explicit NJOYModulesBase(QWidget* parent);
    ~NJOYModulesBase();

public slots:
    virtual void help();
};

