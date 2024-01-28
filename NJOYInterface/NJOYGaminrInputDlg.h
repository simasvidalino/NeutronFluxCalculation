#pragma once

#include <QWidget>

namespace Ui {
class NJOYGaminrInputDlg;
}

class NJOYGaminrInputDlg : public QWidget
{
    Q_OBJECT

public:
    explicit NJOYGaminrInputDlg(QWidget *parent = nullptr);
    ~NJOYGaminrInputDlg();

private:
    Ui::NJOYGaminrInputDlg *ui;
};

