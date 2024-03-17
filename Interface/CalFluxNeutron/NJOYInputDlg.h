#ifndef NJOYINPUTDLG_H
#define NJOYINPUTDLG_H

#include <QDialog>

namespace Ui {
class NJOYInputDlg;
}

class NJOYInputDlg : public QDialog
{
    Q_OBJECT

public:
    explicit NJOYInputDlg(QWidget *parent = nullptr);
    ~NJOYInputDlg();

private slots:
    void onHelpRequested();
    void onRunNJOY();

private:
    Ui::NJOYInputDlg *ui;

    void initDlg();

    void setConnections();

    void saveInputFile();
};

#endif // NJOYINPUTDLG_H
