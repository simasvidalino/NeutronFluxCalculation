#ifndef CUSTOMTABLEVIEW_H
#define CUSTOMTABLEVIEW_H

#include <QTableView>

class CustomTableView : public QTableView
{
public:
    CustomTableView(QWidget *parent = nullptr);

    // QWidget interface
protected:
    double originValue = 0.0;
    QModelIndex originIndex;
    bool dragging = false;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // CUSTOMTABLEVIEW_H
