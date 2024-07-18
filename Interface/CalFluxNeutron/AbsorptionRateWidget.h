#ifndef ABSORPTIONRATEWIDGET_H
#define ABSORPTIONRATEWIDGET_H

#include <QWidget>

namespace Ui {
class AbsorptionRateWidget;
}

class AbsorptionRateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AbsorptionRateWidget(QWidget *parent = nullptr);
    ~AbsorptionRateWidget();

private:
    Ui::AbsorptionRateWidget *ui;
};

#endif // ABSORPTIONRATEWIDGET_H
