#include "CustomComboBox.h"

CustomComboBox::CustomComboBox(QWidget *parent) : QComboBox(parent)
{

}

CustomComboBox::~CustomComboBox()
{
}

void CustomComboBox::focusOutEvent(QFocusEvent *event)
{
    qInfo()<<"focusOutEvent";
    this->clearFocus();

    QWidget * parent = nullptr;
    parent = this->parentWidget();

    if (nullptr != parent)
        parent->setFocus();

    QComboBox::focusOutEvent(event);
}

void CustomComboBox::leaveEvent(QEvent *event)
{
    qInfo()<<"leaveEvent";
    this->clearFocus();

    QWidget * parent = nullptr;
    parent = this->parentWidget();

    if (nullptr != parent)
        parent->setFocus();

    QComboBox::leaveEvent(event);
}
