#include "NJOYStackWidget.h"
#include "ui_NJOYStackWidget.h"

#include "NJOYModulesBase.h"

NJOYStackWidget::NJOYStackWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NJOYStackWidget)
{
    ui->setupUi(this);

    setConnections();
}

NJOYStackWidget::~NJOYStackWidget()
{
    delete ui;
}

void NJOYStackWidget::addWidget(QWidget *widget, const QString moduleName)
{
    ui->stackedWidget->addWidget(widget);

    matNumbers.push_back(moduleName);

    ui->labelMaterial->setText(matNumbers.at(0));
}

void NJOYStackWidget::setCurrentIndex(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

QWidget *NJOYStackWidget::getWidget(int index)
{
    return ui->stackedWidget->widget(index);
}

void NJOYStackWidget::help()
{
    const auto helpWidget = dynamic_cast<NJOYModulesBase*>(ui->stackedWidget->widget(0));

    if (nullptr != helpWidget)
        helpWidget->help();
}

void NJOYStackWidget::setConnections()
{
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(ui->commandLinkButtonBefore, &QCommandLinkButton::clicked,
            ui->stackedWidget, [&](){
        int index = ui->stackedWidget->currentIndex() - 1;

        if (index >= 0)
        {
            ui->stackedWidget->setCurrentIndex(index);
            ui->labelMaterial->setText(matNumbers.at(index));
        }
    });
    connect(ui->commandLinkButtonNext, &QCommandLinkButton::clicked,
            ui->stackedWidget, [&](){
        int index = ui->stackedWidget->currentIndex() + 1;
        if (index < ui->stackedWidget->count())
        {
            ui->stackedWidget->setCurrentIndex(index);
            ui->labelMaterial->setText(matNumbers.at(index));
        }
        });

    connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &NJOYStackWidget::help);
}
