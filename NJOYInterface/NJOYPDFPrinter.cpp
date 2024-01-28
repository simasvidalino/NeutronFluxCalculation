#include "NJOYPDFPrinter.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QPainter>
#include <QScreen>

NJOYPDFPrinter::NJOYPDFPrinter() : QPrinter(PrinterMode::HighResolution)
{
}

NJOYPDFPrinter::~NJOYPDFPrinter()
{

}

QString NJOYPDFPrinter::getDocumentPath() const
{
    return documentPath;
}

void NJOYPDFPrinter::savePDF()
{
    documentPath = QFileDialog::getSaveFileName(nullptr,
                                "Save File",
                                 QDir::homePath(),
                                 "Files (*.pdf *.PDF)");

    printPDF();
}

void NJOYPDFPrinter::printPDF()
{
    if (documentPath.isEmpty())
        documentPath = QDir::currentPath() + "/output.pdf";

    this->setOutputFileName(documentPath);
    this->setOutputFormat(OutputFormat::PdfFormat);

    QPainter painter;

    if (!painter.begin(this))
    {
        qInfo()<<"Erro opening PDF";
        return;
    }

    //painter.drawPixmap()

    painter.drawText(QPointF(0,0), "NJOY Output");

    this->newPage();

    painter.end();
}
