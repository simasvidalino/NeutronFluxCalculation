#ifndef NJOYGENERALPARAMETERS_H
#define NJOYGENERALPARAMETERS_H

#include <QWidget>

namespace Ui {
class NJOYGeneralParameters;
}

class NJOYGeneralParameters : public QWidget
{
    Q_OBJECT

public:
    explicit NJOYGeneralParameters(QWidget *parent = nullptr);
    ~NJOYGeneralParameters();

private:
    Ui::NJOYGeneralParameters *ui;
};

#endif // NJOYGENERALPARAMETERS_H
