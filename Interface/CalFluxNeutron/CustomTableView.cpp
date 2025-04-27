#include "CustomTableView.h"
#include <QMouseEvent>

CustomTableView::CustomTableView(QWidget *parent)
{

}

void CustomTableView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QModelIndex index = this->indexAt(event->pos());

        if (index.isValid() && this->selectionModel()->isSelected(index))
        {
            dragging    = true;
            originIndex = index;
            originValue = index.data(Qt::EditRole).toFloat();
        }
    }

    QTableView::mousePressEvent(event);
}

void CustomTableView::mouseReleaseEvent(QMouseEvent *event)
{
    dragging = false;

    QTableView::mouseReleaseEvent(event);
}

void CustomTableView::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragging)
    {
        return;
    }

    QModelIndex currentIndex = this->indexAt(event->pos());

    if (currentIndex.isValid() && currentIndex != originIndex)
    {
        this->model()->setData(currentIndex, originValue, Qt::EditRole);
    }

    QTableView::mouseMoveEvent(event);
}
