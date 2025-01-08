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
    void on_DecksButton_clicked();
    void on_CreateDeckButton_clicked();
    void on_AddCardButton_clicked();
    void on_SetDescriptionButton_clicked();

    void on_actionUsers_triggered();
    void on_actionFullscreen_triggered();
    void onRowHovered(int row);
    void onRowLeft(int row);

protected:
    void resizeEvent(QResizeEvent *event) override; // Override resizeEvent

private:
    Ui::MainWindow *ui; // Use raw pointer instead of unique_ptr
    bool isContextMenuActive = false;

    void populateTableWidget(const std::vector<Deck>& decks);
    void adjustTableColumnWidths(); // Function to adjust column widths
    void showDeckSettings(const Deck& deck, const int row);

    void setButtonVisibility(int row, bool visible);

    // Helper Methods
    void showDeckInfo(const Deck& deck);
    void insertTableRow(const Deck& deck, const int& row, const bool& insert_default_values);
    bool updateTableRow(const QString& id);
};

#endif // MAINWINDOW_H
