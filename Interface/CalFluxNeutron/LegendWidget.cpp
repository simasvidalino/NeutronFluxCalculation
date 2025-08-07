#include "LegendWidget.h"

#include <QLabel>
#include <QVBoxLayout>

LegendWidget::LegendWidget(QChart *chart, QWidget *parent) : QDialog(parent)
{
    setModal(false);

    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setAlignment(Qt::AlignTop);

    // Add legend
    for (QLegendMarker *marker : chart->legend()->markers())
    {
        QWidget *item = new QWidget();
        auto *itemLayout = new QHBoxLayout(item);
        itemLayout->setSpacing(5);
        itemLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *icon = new QLabel();
        icon->setFixedSize(14, 14);
        icon->setStyleSheet(QString("background-color: %1; border: 1px solid black;")
                                .arg(marker->brush().color().name()));

        QLabel *text = new QLabel(marker->series()->name());

        itemLayout->addWidget(icon, 0, Qt::AlignVCenter);
        itemLayout->addWidget(text, 1, Qt::AlignVCenter);

        contentLayout->addWidget(item);
    }

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(content);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);
}

LegendWidget::~LegendWidget()
{
}
