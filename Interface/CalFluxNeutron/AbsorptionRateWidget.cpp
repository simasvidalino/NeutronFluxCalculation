#include "AbsorptionRateWidget.h"
#include "ui_AbsorptionRateWidget.h"

AbsorptionRateWidget::AbsorptionRateWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AbsorptionRateWidget)
{
    ui->setupUi(this);
}

AbsorptionRateWidget::~AbsorptionRateWidget()
{
    delete ui;
}
