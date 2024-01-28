#pragma once

#include <QtPrintSupport/QPrinter>

class NJOYPDFPrinter : public QPrinter
{
public:
    NJOYPDFPrinter();
    ~NJOYPDFPrinter();

    QString getDocumentPath() const;

    void savePDF();

private:
    void printPDF();

    QString documentPath;
};

