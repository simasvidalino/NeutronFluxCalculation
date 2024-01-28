//#include "NJOYPDFViewDlg.h"

//#include <QPdfPageNavigator>
//#include <QHBoxLayout>
//#include <QPushButton>

//NJOYPDFViewDlg::NJOYPDFViewDlg(QWidget *parent):QPdfView(parent),
//    m_zoomSelector(new QComboBox(this)),
//    m_document(new QPdfDocument(this)),
//    vLayout(new QVBoxLayout(this))
//{
//    setNavigator();

//    auto open = new QPushButton("Open");

//    vLayout->addWidget(open, 1000);

//    connect(open, &QPushButton::clicked, this, &NJOYPDFViewDlg::openPDF);


//}

//NJOYPDFViewDlg::~NJOYPDFViewDlg()
//{

//}

//void NJOYPDFViewDlg::setConnections()
//{
//    //    connect(m_pageSelector, &QSpinBox::valueChanged, this, &NJOYPDFViewDlg::pageSelected);

//    //    auto nav = this->pageNavigator();

//    //    connect(nav, &QPdfPageNavigator::currentPageChanged, m_pageSelector, &QSpinBox::setValue);
//    //    connect(nav, &QPdfPageNavigator::backAvailableChanged, ui->actionBack, &QAction::setEnabled);
//    //    connect(nav, &QPdfPageNavigator::forwardAvailableChanged, ui->actionForward, &QAction::setEnabled);


//}

//void NJOYPDFViewDlg::setZoomSelector()
//{
//}

//void NJOYPDFViewDlg::setNavigator()
//{
//    auto previous = new QPushButton();
//    auto next     = new QPushButton();

//    auto goToPage = new QSpinBox();

//    auto layout = new QHBoxLayout(this);

//    layout->addWidget(previous);
//    layout->addWidget(goToPage);
//    layout->addWidget(next);

//    vLayout->addLayout(layout);
//}

//void NJOYPDFViewDlg::openPDF()
//{
//    auto fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
//                                                 "/home", "PDF (*.pdf *.PDF)");

//    if (fileName.isEmpty())
//        return;

//    m_document->load(fileName);
//    const auto documentTitle = m_document->metaData(QPdfDocument::MetaDataField::Title).toString();
//    //setWindowTitle(!documentTitle.isEmpty() ? documentTitle : QStringLiteral("PDF Viewer"));

//    m_pageSelector->setMaximum(m_document->pageCount() - 1);
//}
