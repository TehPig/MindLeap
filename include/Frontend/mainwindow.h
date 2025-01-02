#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Backend/Classes/Deck.hpp"
#include "Frontend/hoverabletablewidget.h"

#include <vector>

#include <QMainWindow>
#include <QListWidget>
#include <QTableWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QTableWidgetItem>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    public:
        void set_active_widget(std::string widget);
        HoverableTableWidget* get_tableWidget();

    private slots:
        //void on_listWidget_currentRowChanged(int currentRow);
        void onRowHovered(int row);
        void onRowLeft(int row);
        //void onTableItemClicked(QTableWidgetItem *item);
        void showDeckSettings(QPushButton *button);

        bool eventFilter(QObject *obj, QEvent *event);

        void on_pushButton_clicked();
        void on_pushButton_5_clicked();
        void on_pushButton_6_clicked();

        // Action Buttons
        void on_actionUsers_triggered();
        void on_actionFullscreen_triggered();

    private:
        Ui::MainWindow *ui;
        void populateTableWidget(const std::vector<Deck> &decks);
        QList<QPushButton*> settingsButtons;
};

#endif // MAINWINDOW_H
