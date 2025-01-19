#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qpushbutton.h>
#include <vector>

#include <QMainWindow>
#include <QWidget>
#include <QResizeEvent>

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

    QString getCurrentDeck() const;
    void setCurrentDeck(const QString &deckID);

private slots:
    void on_DecksButton_clicked();
    void on_CreateDeckButton_clicked();
    void on_AddCardButton_clicked();
    void on_SetDescriptionButton_clicked();

    void on_actionUsers_triggered();
    void on_actionFullscreen_triggered();
    void onRowHovered(int row);
    void onRowLeft(int row);

    void startStudySession(const QString& deckID);

    void on_actionStudy_Deck_triggered();

    void on_actionPreferences_triggered();

    void on_actionGuide_triggered();

    void on_actionAbout_triggered();

    void on_StudyButton_clicked();

    void on_GetAnswerButton_clicked();

    void on_StatsButton_clicked();

    void on_EndStudyButton_clicked();

protected:
    void resizeEvent(QResizeEvent *event) override; // Override resizeEvent

private:
    Ui::MainWindow *ui; // Use raw pointer instead of unique_ptr
    bool isContextMenuActive = false;

    void populateTableWidget(const std::vector<Deck>& decks);
    void adjustTableColumnWidths(); // Function to adjust column widths
    void showDeckSettings(const Deck& deck, const int row);

    void setButtonVisibility(int row, bool visible);

    // Button Listeners for Study seesion
    void onButtonOptionSelected(QPushButton* button, Card card);

    // Helper Methods
    void showDeckInfo(const Deck& deck);
    void insertTableRow(const Deck& deck, const int& row, const bool& insert_default_values);
    bool updateTableRow(const QString& id);

    void proceedToNextCard(Deck& deck);

    QString currentDeck;
};

#endif // MAINWINDOW_H
