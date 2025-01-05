#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

#include <QComboBox>

class CustomComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit CustomComboBox(QWidget* parent = nullptr);

protected:
    virtual void focusOutEvent(QFocusEvent *event) override;

    // QWidget interface
protected:
    virtual void leaveEvent(QEvent *event) override;
};

#endif
