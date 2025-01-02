#ifndef HOVERABLETABLEWIDGET_H
#define HOVERABLETABLEWIDGET_H

#include <QTableWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QPoint>

class HoverableTableWidget : public QTableWidget {
    Q_OBJECT

public:
    explicit HoverableTableWidget(QWidget *parent = nullptr);

signals:
    void rowHovered(int row); // Emitted when a new row is hovered
    void rowLeft(int row);    // Emitted when the mouse leaves a row

protected:
    bool eventFilter(QObject *obj, QEvent *event) override; // Event filter for mouse events
    int lastHoveredRow; // Stores the last hovered row index
};

#endif // HOVERABLETABLEWIDGET_H
