#include "Frontend/hoverabletablewidget.h"
#include <QEvent>
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>

HoverableTableWidget::HoverableTableWidget(QWidget *parent)
    : QTableWidget(parent), lastHoveredRow(-1) {
    setMouseTracking(true);
    viewport()->setMouseTracking(true);  // Enable mouse tracking on viewport
    viewport()->installEventFilter(this);  // Install event filter on viewport
}

void HoverableTableWidget::setContextMenuActive(bool active) {
    isContextMenuActive = active;
}

bool HoverableTableWidget::getContextMenuActive() const {
    return isContextMenuActive;
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
                    // Emit rowLeft signal when mouse leaves the row
                    emit rowLeft(lastHoveredRow);
                }
                lastHoveredRow = row;
                if (row != -1) {
                    // Emit rowHovered signal when row is hovered
                    emit rowHovered(row);
                }
            }
        } else if (event->type() == QEvent::Type::Leave) {
            if (lastHoveredRow != -1) {
                // Mouse has left the viewport, emit rowLeft signal
                emit rowLeft(lastHoveredRow);
                lastHoveredRow = -1;
            }
        }
    }
    return QTableWidget::eventFilter(obj, event);
}
