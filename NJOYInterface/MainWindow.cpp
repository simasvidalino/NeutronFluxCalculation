#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include "NJOYInterfaceDefinitions.h"
#include "NJOYEnterDataWizard.h"

#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QStyle>

#include "NJOYPDFViewDlg.h"

#include <QSvgRenderer>
#include <QPainter>
#include <QtSvgWidgets/QtSvgWidgets>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setConnections();

    this->setWindowTitle(appTitle);

    wizard.setParent(this);

    wizard.show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile(const QString &fileName)
{
    bool status = false;

    status = QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));

    qInfo()<<status;
}

void MainWindow::setConnections()
{
    //connect(ui->actionAboutNJOY21, &QAction::triggered, this, [&](){openFile(NJOYManualPath);});
}






