#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>

#include <QMainWindow>

#include "Backend/Classes/Deck.hpp"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    private slots:
        void on_pushButton_clicked();
    void on_listWidget_currentRowChanged(int currentRow);

private:
    Ui::MainWindow *ui;
    void populateListWidget(const std::vector<Deck> &decks);
};

#endif // MAINWINDOW_H