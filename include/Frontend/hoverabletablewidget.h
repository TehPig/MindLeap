#ifndef HOVERABLETABLEWIDGET_H
#define HOVERABLETABLEWIDGET_H

#include <QTableWidget>

class HoverableTableWidget : public QTableWidget {
    Q_OBJECT

public:
    explicit HoverableTableWidget(QWidget *parent = nullptr);
    void setContextMenuActive(bool active); // Public setter method
    bool getContextMenuActive() const; // Public getter method

signals:
    void rowHovered(int row);
    void rowLeft(int row);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    bool isContextMenuActive = false;
    int lastHoveredRow;
};

#endif // HOVERABLETABLEWIDGET_H
