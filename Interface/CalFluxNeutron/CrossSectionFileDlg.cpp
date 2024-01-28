#include "CrossSectionFileDlg.h"
#include "VariablesUsed.h"
#include "ui_CrossSectionFileDlg.h"

#include <QFileDialog>
#include <QTableView>


CrossSectionFileDlg::CrossSectionFileDlg(QWidget *parent,
                                         QStringList materials,
                                         int energyGroup,
                                         int legendreOrder) :
    QDialog(parent),
    energyGroup(energyGroup),
    legendreOrder(legendreOrder),
    materialList(materials),
    ui(new Ui::CrossSectionFileDlg)
{
    ui->setupUi(this);

    initDlg();

    setConnection();
}

CrossSectionFileDlg::~CrossSectionFileDlg()
{
    delete ui;
}

void CrossSectionFileDlg::saveCrossSectionFileDlgCrossSection(long double ****s_s)
{

}

void CrossSectionFileDlg::clearText()
{

}
void CrossSectionFileDlg::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Text Files (*.txt)");

    if (!fileName.isEmpty())
    {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            QString content = stream.readAll();
            file.close();

            //@Todo fill table
        } else
        {
            qWarning() << "Error opening the file for reading.";
        }
    }
}

void CrossSectionFileDlg::saveText()
{
    //QString content = ui->textEdit->toPlainText();

    QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "File Text(txt) (*.txt)");

    if (!fileName.isEmpty())
    {
        QFile file(fileName);

        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            // stream << content;
            file.close();
        }
        else
        {
            qWarning() << "Error opening file for writing";
        }
    }
}

void CrossSectionFileDlg::initDlg()
{
    QStringList horizontalHeaders;
    QString verticalHeader = "Group";
    std::vector<double> values;

    for(int iIndex = 0; iIndex < legendreOrder; ++iIndex)
    {
        horizontalHeaders.append(QString("Legendre") + QString::number(iIndex));
        values.push_back(iIndex);
    }

    for (int index = 0; index < energyGroup; ++index)
    {
        tableView = std::make_unique<QTableView>();
        tableView->setGeometry(0, 0, 400, 300);
        model = std::make_unique<GradesTableModel>();
        model->configTable(3, 5, horizontalHeaders, verticalHeader);
        tableView->setModel(model.get());

        tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        ui->stackedWidget->addWidget(tableView.get()) ;
        tableView->show();
        //Load
    }

     ui->stackedWidget->setCurrentIndex(0);
}

void CrossSectionFileDlg::setConnection()
{
    connect(ui->commandLinkButtonOpen, &QCommandLinkButton::clicked, this, &CrossSectionFileDlg::openFile);
    connect(ui->commandLinkButtonClear, &QCommandLinkButton::clicked, this, &CrossSectionFileDlg::clearText);
    connect(ui->commandLinkButtonSave, &QCommandLinkButton::clicked, this, &CrossSectionFileDlg::saveText);


    connect(ui->commandLinkButtonLast, &QCommandLinkButton::clicked,
            ui->stackedWidget, [&](){
        int index = ui->stackedWidget->currentIndex() - 1;

        if (index >= 0)
        {
            ui->stackedWidget->setCurrentIndex(index);
            this->setWindowTitle(QString("Cross Section ")+ materialList.at(index));
        }
    });
    connect(ui->commandLinkButtonNext, &QCommandLinkButton::clicked,
            ui->stackedWidget, [&](){
        int index = ui->stackedWidget->currentIndex() + 1;
        if (index < ui->stackedWidget->count())
        {
           this->setWindowTitle(QString("Cross Section ") +  materialList.at(index));
        }
        });
}
