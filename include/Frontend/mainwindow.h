#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QResizeEvent>
#include <vector>
#include "Backend/Classes/Deck.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class HoverableTableWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    HoverableTableWidget* get_tableWidget();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_actionUsers_triggered();
    void on_actionFullscreen_triggered();
    void onRowHovered(int row);
    void onRowLeft(int row);

protected:
    void resizeEvent(QResizeEvent *event) override; // Override resizeEvent

private:
    void populateTableWidget(const std::vector<Deck>& decks);
    //void setupTableWidget();
    void showDeckSettings(const Deck& deck, const int row);

    void setButtonVisibility(int row, bool visible);

    //void resizeEvent(QResizeEvent *event) override;

    Ui::MainWindow *ui; // Use raw pointer instead of unique_ptr
    bool isContextMenuActive = false;

    void adjustTableColumnWidths(); // Function to adjust column widths
};

#endif // MAINWINDOW_H
