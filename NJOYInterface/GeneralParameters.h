#ifndef GENERALPARAMETERS_H
#define GENERALPARAMETERS_H

#include <QWidget>

namespace Ui {
class GeneralParameters;
}

class GeneralParameters : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralParameters(QWidget *parent = nullptr);
    ~GeneralParameters();

private:
    Ui::GeneralParameters *ui;
};

#endif // GENERALPARAMETERS_H
