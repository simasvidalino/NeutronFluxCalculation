#ifndef REGININFORMATIONDLG_H
#define REGININFORMATIONDLG_H

#include <QDialog>

namespace Ui {
class ReginInformationDlg;
}

class ReginInformationDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ReginInformationDlg(QWidget *parent = nullptr);
    ~ReginInformationDlg();

private:
    Ui::ReginInformationDlg *ui;
};

#endif // REGININFORMATIONDLG_H
