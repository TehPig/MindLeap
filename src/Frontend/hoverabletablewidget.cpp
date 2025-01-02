#include "Frontend/hoverabletablewidget.h"
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

HoverableTableWidget::HoverableTableWidget(QWidget *parent)
    : QTableWidget(parent), lastHoveredRow(-1) {
    setMouseTracking(true);
    viewport()->setMouseTracking(true); // Enable mouse tracking on viewport
    viewport()->installEventFilter(this); // Install event filter on viewport
}

bool HoverableTableWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == viewport()) {
        if (event->type() == QEvent::Type::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QPoint pos = mouseEvent->position().toPoint();
            QModelIndex index = indexAt(pos);
            int row = index.isValid() ? index.row() : -1;

            if (row != lastHoveredRow) {
                if (lastHoveredRow != -1) {
                    //qDebug() << "Emitting rowLeft for row:" << lastHoveredRow;
                    emit rowLeft(lastHoveredRow);
                }
                lastHoveredRow = row;
                if (row != -1) {
                    //qDebug() << "Emitting rowHovered for row:" << row;
                    emit rowHovered(row);
                }
            }
        } else if (event->type() == QEvent::Type::Leave) {
            if (lastHoveredRow != -1) {
                //qDebug() << "Leave event, emitting rowLeft for row:" << lastHoveredRow;
                emit rowLeft(lastHoveredRow);
                lastHoveredRow = -1;
            }
        }
    }
    return QTableWidget::eventFilter(obj, event);
}
