#include "NJOYWizard.h"

NJOYWizard::NJOYWizard(QWidget *parent)
    :QWizard{parent}
{
    this->setOption(QWizard::HaveHelpButton, true);

}
