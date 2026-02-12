#include <QSqlQuery>
#include <QTableWidget>
#include <QIcon>
#include <QTimer>
#include <QSize>
#include <QStyle>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QScrollBar>
#include <QCursor>
#include <QSoundEffect>
#include <QUrl>
#include "Backend/Utilities/Logger.hpp"
#include "Backend/Utilities/DiscordManager.hpp"
#include "Backend/Classes/User.hpp"
#include "Backend/Database/setup.hpp"
#include "Frontend/mainwindow.h"
#include "Frontend/Dialogs/preferencesdialog.h"
#include "Frontend/Dialogs/statsdialog.h"
#include "Frontend/Dialogs/studydeckdialog.h"
#include "Frontend/invalidinputbox.h"
#include "Frontend/selectuser.h"
#include "Frontend/hoverabletablewidget.h"
#include "Frontend/Dialogs/addcarddialog.h"
#include "Frontend/Dialogs/confirmationdialog.h"
#include "Frontend/Dialogs/customdialog.h"
#include "Frontend/Dialogs/aboutdialog.h"
#include "Frontend/Dialogs/guidedialog.h"
#include "forms/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow()) {
    ui->setupUi(this);

    setWindowIcon(QIcon(":/assets/images/app_icon.ico"));

    // Initialize sound once
    popSound = new QSoundEffect(this);
    popSound->setSource(QUrl("qrc:/assets/sounds/pop.wav"));
    popSound->setVolume(0.5f);

    // Make sure unwanted elements are hidden
    ui->scrollArea->setVisible(true);
    ui->AddCardButton->setVisible(false);
    ui->deckWidget->setVisible(false);
    ui->study->setVisible(false);
    ui->studyFinished->setVisible(false);
    ui->BrowseButton->setVisible(false);
    ui->SetDescriptionButton->setVisible(false);
    ui->EndStudyButton->setVisible(false);

    // Make the table widget hoverable
    auto *tableWidget = qobject_cast<HoverableTableWidget*>(ui->CardList);
    connect(tableWidget, &HoverableTableWidget::rowHovered, this, &MainWindow::onRowHovered);
    connect(tableWidget, &HoverableTableWidget::rowLeft, this, &MainWindow::onRowLeft);

    // Once DB is initialized, fetch the current user
    auto db = Database::getInstance("app_data.db");
    if(!db->getDB().isOpen()) db->initialize();

    User user;
    const bool status = user.fetchSelected();
    if(!status){
        if(!user.create()){
            Logger::error("Default User could not be created", "Main");
            return;
        }
        user.select();
    }

    Logger::info(QString("Session started for user: %1").arg(user.getUsername()), "Main");
    setWindowTitle(user.getUsername() + " | MindLeap");

    if(user.listDecks().empty()){
        Deck deck;
        deck.create();
    }

    user.updateLaunchStats();

    DiscordManager::updatePresence("Browsing Decks", "", "browse");
    populateTableWidget(user.listDecks());

    // Pre-initialize heavy dialogs in the background to make them instant on first click
    QTimer::singleShot(100, this, [this]() {
        if (!guideDialog) {
            guideDialog = new GuideDialog(this);
            guideDialog->setModal(true);
            
            // Warm up: Force window creation and layout calculation off-screen
            guideDialog->move(-10000, -10000);
            guideDialog->show();
            guideDialog->hide();
        }
        if (!aboutDialog) {
            aboutDialog = new AboutDialog(this);
            aboutDialog->setModal(true);
            
            // Warm up
            aboutDialog->move(-10000, -10000);
            aboutDialog->show();
            aboutDialog->hide();
        }
    });
}

MainWindow::~MainWindow() {
    if (guideDialog) delete guideDialog;
    if (aboutDialog) delete aboutDialog;
    delete ui;
}

HoverableTableWidget* MainWindow::get_tableWidget() {
    return static_cast<HoverableTableWidget*>(ui->CardList);
}

void MainWindow::populateTableWidget(const std::vector<Deck>& decks) {
    ui->CardList->setRowCount(decks.size());
    for(int row = 0; row < decks.size(); row++) insertTableRow(decks[row], row, false);

    QTimer::singleShot(0, this, [this]() {
        adjustTableColumnWidths();
    });
}

void MainWindow::adjustTableColumnWidths() {
    QTableWidget* table = ui->CardList;
    ui->CardList->setFocusPolicy(Qt::NoFocus);

    // Total available width
    double totalWidth = static_cast<double>(table->viewport()->width());

    // Subtract the width of the vertical scrollbar if it is visible
    if (table->verticalScrollBar()->isVisible()) {
        totalWidth -= static_cast<double>(table->verticalScrollBar()->width());
    }

    // Proportions for each column except the last one
    const double proportions[] = {0.5, 0.1, 0.1, 0.1, 0.1};
    const int columnCount = sizeof(proportions) / sizeof(proportions[0]);

    // Verify proportions sum to less than 1.0
    double sum = 0;
    for (double proportion : proportions) {
        sum += proportion;
    }
    Q_ASSERT(sum < 1.0);

    // Set the column widths for all columns except the last one
    qDebug() << "====================================";
    qDebug() << "[DeckWidget] Setting Column sizes";
    for (int i = 0; i < columnCount; ++i) {
        double width = totalWidth * proportions[i];

        // Calculate the minimum width needed for the text in the first row
        if (table->rowCount() > 0) {
            QTableWidgetItem* firstRowItem = table->item(0, i);
            if (firstRowItem) {
                double contentWidth = static_cast<double>(table->fontMetrics().horizontalAdvance(firstRowItem->text())) + 30.0; // Add padding for readability
                width = qMax(width, contentWidth);  // Use the larger of calculated width or content width
            }
        }

        // Calculate the minimum width needed for the header text
        QString headerText = table->horizontalHeaderItem(i)->text();
        double headerWidth = static_cast<double>(table->fontMetrics().horizontalAdvance(headerText)) + 30.0; // Add padding for readability
        width = qMax(width, headerWidth);  // Use the larger of calculated width or header width

        // Apply the adjusted width
        table->setColumnWidth(i, static_cast<int>(width));
        qDebug() << "Column" << i << "width set to:" << width;

        table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    }

    // Set the last column to stretch mode
    table->horizontalHeader()->setSectionResizeMode(columnCount, QHeaderView::Stretch);

    // Update the viewport to reflect changes
    table->viewport()->update();
}

// Override resizeEvent to adjust column widths on resize
void MainWindow::resizeEvent(QResizeEvent *event) {
    if(!ui->CardList->isVisible()) return;
    QMainWindow::resizeEvent(event); // Call base class implementation
    adjustTableColumnWidths();       // Adjust column widths dynamically
}

void MainWindow::setButtonVisibility(int row, bool visible) {
    auto *container = ui->CardList->cellWidget(row, 5);
    if (container && !ui->CardList->getContextMenuActive()) {
        container->findChild<QPushButton *>()->setVisible(visible);
    }
}

// Context menu function that opens on settings button click
void MainWindow::showDeckSettings(const Deck& deck, const int row) {
    auto *tableWidget = qobject_cast<HoverableTableWidget*>(ui->CardList);
    tableWidget->setContextMenuActive(true);

    QMenu contextMenu(this);

    auto *action1 = new QAction("Rename", this);
    connect(action1, &QAction::triggered, this, [this, tableWidget, row]() {
            QTableWidgetItem* nameItem = tableWidget->item(row, 0);
            QLabel* nameLabel = qobject_cast<QLabel*>(tableWidget->cellWidget(row, 0));
            if (nameItem) {
                // Retrieve the stored ID from the UserRole data
                QVariant idVariant = nameItem->data(Qt::UserRole);
                if (idVariant.isValid()) {
                    QString deckID = idVariant.toString();
                    auto *container = tableWidget->cellWidget(row, 5);
                    if (container) container->findChild<QPushButton *>()->setVisible(false);

                    // Set context menu inactive
                    tableWidget->setContextMenuActive(false);

                    // Create the confirmation dialog
                    CustomDialog* renameDialog = new CustomDialog(this);
                    renameDialog->setWindowTitleText("Rename Deck");
                    renameDialog->setMessageText("Enter new Deck name:");
                    if (renameDialog->exec() == QDialog::Accepted) {
                        QString name = renameDialog->getEnteredText();
                        if(name.isEmpty()) {
                            this->statusBar()->showMessage("Error: Deck name cannot be empty");
                            return;
                        }

                        Deck deck(deckID);
                        const bool status = deck.rename(name);
                        if(!status) {
                            this->statusBar()->showMessage("Error: Could not rename Deck.");
                            return;
                        }

                        nameLabel->setText("<a href=\"#\">" + name + "</a>");
                    }
                }
            }
    });
    contextMenu.addAction(action1);

    auto *action2 = new QAction("Delete", this);
    connect(action2, &QAction::triggered, this, [this, tableWidget, row]() {
            QTableWidgetItem* nameItem = tableWidget->item(row, 0);
            if (nameItem) {
                QVariant idVariant = nameItem->data(Qt::UserRole);
                if (idVariant.isValid()) {
                    QString deckID = idVariant.toString();

                    auto* container = tableWidget->cellWidget(row, 5);
                    if (container) {
                        container->findChild<QPushButton*>()->setVisible(false);
                    }

                    tableWidget->setContextMenuActive(false);

                    ConfirmationDialog* deleteDialog = new ConfirmationDialog("delete this deck", this);
                    if (deleteDialog->exec() == QDialog::Accepted) {
                        Deck deck(deckID);
                        if (!deck._delete()) {
                            this->statusBar()->showMessage("Error: Could not delete Deck.");
                            return;
                        }
                        tableWidget->removeRow(row);  // Use dynamically retrieved row
                    }
                }
            }
    });
    contextMenu.addAction(action2);

    // Connect the aboutToHide signal to reset the flag
    connect(&contextMenu, &QMenu::aboutToHide, this, [this, tableWidget, row]() {
        if (row != -1) {
            auto *container = tableWidget->cellWidget(row, 5);
            if (container) container->findChild<QPushButton *>()->setVisible(false);
        }
        tableWidget->setContextMenuActive(false);
    });

    // Show the context menu
    contextMenu.exec(QCursor::pos());
}

void MainWindow::onRowHovered(int row) { setButtonVisibility(row, true); }

void MainWindow::onRowLeft(int row) { setButtonVisibility(row, false); }

// Dialogs
void MainWindow::on_CreateDeckButton_clicked() {
    CustomDialog dialog(this);
    dialog.setWindowTitleText("Create Deck");
    dialog.setMessageText("Enter Deck name:");

    if (dialog.exec() == QDialog::Accepted) {
        QString deckName = dialog.getEnteredText(); // Now safe to get entered text

        Deck deck("n_" + deckName);
        const bool status = deck.create();
        if(!status){
            this->statusBar()->showMessage("Error: Deck was not created.");
            return;
        }

        QTableWidget* list = this->get_tableWidget();
        int row = list->rowCount();
        list->insertRow(row);

        insertTableRow(deck, row, true);

        ui->CardList->viewport()->update();

        // Show hidden buttons
        ui->CreateDeckButton->setVisible(false);
        ui->SetDescriptionButton->setVisible(true);

        showDeckInfo(deck);
    }
}

void MainWindow::on_DecksButton_clicked() {
    DiscordManager::updatePresence("Browsing Decks", "", "browse");

    if(!ui->scrollArea->isVisible()){
        ui->study->setVisible(false);
        ui->studyFinished->setVisible(false);
        ui->deckWidget->setVisible(false);
        ui->scrollArea->setVisible(true);

        // Change button visibility
        ui->CreateDeckButton->setVisible(true);
        ui->SetDescriptionButton->setVisible(false);
        ui->EndStudyButton->setVisible(false);

        adjustTableColumnWidths();
    }
}

void MainWindow::on_SetDescriptionButton_clicked() {
    CustomDialog dialog(this);
    dialog.setWindowTitleText("Set Deck Description");
    dialog.setMessageText("Enter description:");

    if(dialog.exec() == QDialog::Accepted){
        QString deckDescription = dialog.getEnteredText(); // Now safe to get entered text

        const QString id = ui->Name->property("deckID").toString();
        Deck deck(id);
        const bool status = deck.setDescription(deckDescription);
        if(!status){
            this->statusBar()->showMessage("Error: Deck Description was not set.");
            return;
        }

        if(!ui->scrollArea_2->isVisible()) ui->scrollArea_2->setVisible(true);

        ui->Description->setText(deckDescription);
    }
}

void MainWindow::on_AddCardButton_clicked() {
    AddCardDialog dialog(this);

    if(dialog.exec() == QDialog::Accepted){
        const QString id = ui->Name->property("deckID").toString();
        const QString question = dialog.getQuestion();
        const QString answer = dialog.getAnswer();

        Card card(question, answer);
        Deck deck(id);
        const bool status = deck.addCard(card);
        if(!status){
            this->statusBar()->showMessage("Error: Card not added to Deck");
            return;
        }

        // In order to display the name deck.fetch will be needed
        deck.fetch();

        // Fire the function to display the deck information, a quick but not the best way to do so.
        showDeckInfo(deck);

        // Card added to deck so the study button can be shown
        if(!ui->StudyButton->isVisible()) ui->StudyButton->setVisible(true);
    }
}

// Helper Methods
void MainWindow::showDeckInfo(const Deck& deck) {
    ui->CreateDeckButton->setVisible(false);
    ui->SetDescriptionButton->setVisible(true);
    ui->AddCardButton->setVisible(true);

    if(deck.getCardCount() == 0) ui->StudyButton->setVisible(false);

    // Change visible widget
    ui->scrollArea->setVisible(false);
    ui->deckWidget->setVisible(true);

    // Ensure the QLabel widgets are visible
    // Set the text
    ui->Name->setText(deck.getName());
    ui->Name->setProperty("deckID", deck.getID());

    const QString deckDescription = deck.getDescription();
    if(!deckDescription.isEmpty()){
        ui->scrollArea_2->setVisible(true);
        ui->Description->setText(deckDescription);
    } else ui->scrollArea_2->setVisible(false);

    const std::vector<int> card_count = deck.getCardInformation();
    if(card_count.empty()){
        this->statusBar()->showMessage("Error: Could not retrieve card counts");
        return;
    }

    if(!card_count.empty()){
        ui->UnseenCount->setText(QString::number(card_count[0]));
        ui->PendingCount->setText(QString::number(card_count[1]));
        ui->ReviewCount->setText(QString::number(card_count[2]));

        // Hide study button if no cards are available within limits
        if (card_count[0] == 0 && card_count[1] == 0 && card_count[2] == 0) {
            ui->StudyButton->setVisible(false);
        } else {
            ui->StudyButton->setVisible(true);
        }
    }

    ui->CardCount->setText("Total Cards: " + QString::number(deck.getCardCount()));

    ui->Description->adjustSize();
    // ui->UnseenCount->adjustSize();
    // ui->ReviewCount->adjustSize();
    // ui->PendingCount->adjustSize();
}

void MainWindow::insertTableRow(const Deck& deck, const int& row, const bool& insert_default_values){
    QFont font;
    font.setPointSize(12);

    // Set Names
    QLabel *nameLabel = new QLabel();
    nameLabel->setText("<a href=\"#\">" + deck.getName() + "</a>");
    nameLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    nameLabel->setOpenExternalLinks(false);

    nameLabel->setStyleSheet(R"(
        QLabel {
            background-color: transparent;
            text-decoration: none;
        }
        QLabel::hover {
            text-decoration: underline;
        }
    )");

    connect(nameLabel, &QLabel::linkActivated, this, [this, deck]() mutable {
        Deck nonConstDeck = deck; // Create a non-const copy of the deck

        if (nonConstDeck.fetch()){
            DiscordManager::updatePresence("Managing Deck", deck.getName(), "deck");
            showDeckInfo(nonConstDeck);
        }
        else this->statusBar()->showMessage("Error: Could not fetch deck information.");
    });

    ui->CardList->setCellWidget(row, 0, nameLabel);

    QTableWidgetItem *nameItem = new QTableWidgetItem();
    nameItem->setData(Qt::UserRole, QVariant(deck.getID()));
    ui->CardList->setItem(row, 0, nameItem);

    std::vector<int> card_count;
    if(!insert_default_values){
        card_count = deck.getCardInformation();
        if(card_count.empty()){
            this->statusBar()->showMessage("Error: Card Counts could not be loaded.");
            return;
        }
    }

    // Set New, Review, and Pending values
    auto *newItem = new QTableWidgetItem(!insert_default_values ? QString::number(card_count[0]) : "0");
    newItem->setFont(font);
    newItem->setTextAlignment(Qt::AlignCenter);
    newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable); // Make non-editable
    ui->CardList->setItem(row, 1, newItem);

    auto *reviewItem = new QTableWidgetItem(!insert_default_values ? QString::number(card_count[2]) : "0");
    reviewItem->setFont(font);
    reviewItem->setTextAlignment(Qt::AlignCenter);
    reviewItem->setFlags(reviewItem->flags() & ~Qt::ItemIsEditable); // Make non-editable
    ui->CardList->setItem(row, 2, reviewItem);

    auto *learnItem = new QTableWidgetItem(!insert_default_values ? QString::number(card_count[1]) : "0");
    learnItem->setFont(font);
    learnItem->setTextAlignment(Qt::AlignCenter);
    learnItem->setFlags(learnItem->flags() & ~Qt::ItemIsEditable); // Make non-editable
    ui->CardList->setItem(row, 3, learnItem);

    auto *totalItem = new QTableWidgetItem(!insert_default_values ? QString::number(deck.getCardCount()) : "0");
    totalItem->setFont(font);
    totalItem->setTextAlignment(Qt::AlignCenter);
    totalItem->setFlags(totalItem->flags() & ~Qt::ItemIsEditable); // Make non-editable
    ui->CardList->setItem(row, 4, totalItem);

    // Settings Button in Container (Hidden by default)
    auto *settingsButton = new QPushButton();
    settingsButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_CommandLink));
    settingsButton->setIconSize(QSize(32, 32));
    settingsButton->setStyleSheet(R"(QPushButton {
                                            border: none;
                                            background: transparent;
                                            color: #a9b7c6;
                                        }
                                        QPushButton:focus {
                                            outline: none;
                                        }
                                    )");

    settingsButton->setFocusPolicy(Qt::NoFocus);  // Avoid focus interference
    settingsButton->setVisible(false);

    // Connect the settings button to the context menu or other actions
    connect(settingsButton, &QPushButton::clicked, this, [this, deck, row]() {
        showDeckSettings(deck, row);
    });

    // Container for the button in column 5
    auto *container = new QWidget();
    auto *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0); // Remove margins
    layout->addWidget(settingsButton);
    layout->setAlignment(Qt::AlignCenter);
    container->setLayout(layout);

    ui->CardList->setCellWidget(row, 5, container);

    // Install event filter to detect hover and hide/show the button
    container->installEventFilter(this);
}

// bool MainWindow::updateTableRow(const QString& id){
//     for (int row = 0; row < ui->CardList->rowCount(); ++row) {
//         QTableWidgetItem* item = ui->CardList->item(row, 0);
//         if (item && item->data(Qt::UserRole).toString() == id){

//         }
//     }

//     return false;
// }

// Action Buttons
// Select User Menu
void MainWindow::on_actionUsers_triggered() {
    this->close();

    auto *selectUser = new selectuser();
    selectUser->show();
    selectUser->setAttribute(Qt::WA_DeleteOnClose); // Delete select user window on close
}

// Toggle Fullscreen
void MainWindow::on_actionFullscreen_triggered() {
    this->isFullScreen() ? this->showNormal() : this->showFullScreen();
}

// Study Deck Dialog
void MainWindow::on_actionStudy_Deck_triggered() {
    StudyDeckDialog* dialog = new StudyDeckDialog(this);
    connect(dialog, &StudyDeckDialog::studySessionRequested, this, &MainWindow::startStudySession);

    dialog->exec();
    delete dialog;
}

void MainWindow::startStudySession(const QString& deckID) {
    this->currentDeckID = deckID;
    this->currentDeckObj = Deck(deckID);

    if (currentDeckObj.study()) {
        Logger::info("Study session started", QString("DeckID: %1").arg(deckID));

        ui->deckWidget->setVisible(false);
        ui->scrollArea->setVisible(false);
        ui->studyFinished->setVisible(false);
        ui->study->setVisible(true);

        ui->EndStudyButton->setVisible(true);
        ui->CreateDeckButton->setVisible(false);
        ui->SetDescriptionButton->setVisible(false);
        ui->AddCardButton->setVisible(false);

        ui->AgainButton->setVisible(false);
        ui->HardButton->setVisible(false);
        ui->GoodButton->setVisible(false);
        ui->EasyButton->setVisible(false);

        proceedToNextCard();
    } else {
        Logger::info("No cards due for study in deck: " + deckID, "Main");
        showStyledMessageBox("MindLeap", "No cards are currently due for study in this deck.", QMessageBox::Information);
        statusBar()->showMessage("No cards due for study.");
    }
}

// Preferences Dialog
void MainWindow::on_actionPreferences_triggered() {
    PreferencesDialog* dialog = new PreferencesDialog(this);
    connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
    dialog->exec();
}

void MainWindow::on_actionGuide_triggered() {
    if (!guideDialog) {
        guideDialog = new GuideDialog(this);
        guideDialog->setModal(true);
    }
    
    // Center on parent if it was moved off-screen during pre-init
    if (guideDialog->x() < 0) {
        guideDialog->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                guideDialog->size(),
                this->geometry()
            )
        );
    }

    guideDialog->show();
    guideDialog->raise();
    guideDialog->activateWindow();
}

void MainWindow::on_actionAbout_triggered() {
    if (!aboutDialog) {
        aboutDialog = new AboutDialog(this);
        aboutDialog->setModal(true);
    }

    // Center on parent
    if (aboutDialog->x() < 0) {
        aboutDialog->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                aboutDialog->size(),
                this->geometry()
            )
        );
    }

    aboutDialog->show();
    aboutDialog->raise();
    aboutDialog->activateWindow();
}

void MainWindow::on_StudyButton_clicked() {
    // Get deck id
    const QString deckID = ui->Name->property("deckID").toString();
    
    this->currentDeckID = deckID;
    this->currentDeckObj = Deck(deckID);
    
    if (currentDeckObj.study()) {
        Logger::info("Study session started", QString("DeckID: %1").arg(deckID));
        DiscordManager::updatePresence("Studying", currentDeckObj.getName(), "study");

        // Only hide elements if we actually have cards to study
        ui->deckWidget->setVisible(false);
        ui->study->setVisible(true);
        ui->EndStudyButton->setVisible(true);
        ui->SetDescriptionButton->setVisible(false);
        ui->AddCardButton->setVisible(false);
        
        ui->AgainButton->setVisible(false);
        ui->HardButton->setVisible(false);
        ui->GoodButton->setVisible(false);
        ui->EasyButton->setVisible(false);

        proceedToNextCard();
    } else {
        Logger::info("No cards due for study in deck: " + deckID, "Main");
        showStyledMessageBox("MindLeap", "No cards are currently due for study in this deck.", QMessageBox::Information);
        statusBar()->showMessage("No cards due for study.");
    }
}

void MainWindow::proceedToNextCard(){
    // Get Next Card
    this->currentCard = currentDeckObj.getNextCard();

    if(this->currentCard.isEmpty()){
        ui->study->setVisible(false);
        ui->EndStudyButton->setVisible(false);
        ui->studyFinished->setVisible(true);
        DiscordManager::updatePresence("Browsing Decks", "", "browse");

        if(!currentDeckObj.endStudy()){
            Logger::error("Could not end studying session", "Main");
        }

        this->currentDeckID.clear();
        return;
    }

    // Make getAnswer button visible and hide rating buttons
    ui->GetAnswerButton->setVisible(true);
    ui->AgainButton->setVisible(false);
    ui->HardButton->setVisible(false);
    ui->GoodButton->setVisible(false);
    ui->EasyButton->setVisible(false);

    // Ensure study container is visible
    ui->study->setVisible(true);

    // Update Question and Answer
    ui->cardQuestion->setText(this->currentCard.getQuestion());
    ui->cardAnswer->setText(this->currentCard.getAnswer());
    ui->cardAnswer->setVisible(false);

    // Update counters
    const std::vector counters = currentDeckObj.getCardInformation();
    if (!counters.empty()) {
        ui->UnseenCardCount->setText(QString::number(counters[0]));
        ui->PendingCardCount->setText(QString::number(counters[1]));
        ui->ReviewCardCount->setText(QString::number(counters[2]));
    }
}


void MainWindow::on_GetAnswerButton_clicked() {
    // Hide getAnswer button
    ui->GetAnswerButton->setVisible(false);
    
    // Make Prompt buttons visible
    ui->AgainButton->setVisible(true);
    ui->HardButton->setVisible(true);
    ui->GoodButton->setVisible(true);
    ui->EasyButton->setVisible(true);

    // Make answer visible
    ui->cardAnswer->setVisible(true);

    disconnect(ui->AgainButton, nullptr, this, nullptr);
    disconnect(ui->HardButton, nullptr, this, nullptr);
    disconnect(ui->GoodButton, nullptr, this, nullptr);
    disconnect(ui->EasyButton, nullptr, this, nullptr);

    // Add button click listeners
    connect(ui->AgainButton, &QPushButton::clicked, this, [this]() {
        onButtonOptionSelected(ui->AgainButton);
    });
    connect(ui->HardButton, &QPushButton::clicked, this, [this]() {
        onButtonOptionSelected(ui->HardButton);
    });
    connect(ui->GoodButton, &QPushButton::clicked, this, [this]() {
        onButtonOptionSelected(ui->GoodButton);
    });
    connect(ui->EasyButton, &QPushButton::clicked, this, [this]() {
        onButtonOptionSelected(ui->EasyButton);
    });
}

void MainWindow::onButtonOptionSelected(QPushButton* button) {
    if (!button) return;

    // Disconnect buttons to prevent multiple clicks
    disconnect(ui->AgainButton, nullptr, this, nullptr);
    disconnect(ui->HardButton, nullptr, this, nullptr);
    disconnect(ui->GoodButton, nullptr, this, nullptr);
    disconnect(ui->EasyButton, nullptr, this, nullptr);

    if (popSound && popSound->isLoaded()) {
        popSound->play();
    }

    if (this->currentCard.isEmpty()) {
        Logger::warn("Attempted to process response for an empty card", "Main");
        proceedToNextCard();
        return;
    }

    QString buttonText = button->text();
    int button_id = 0;
    if (buttonText == "Again") button_id = 1;
    else if (buttonText == "Hard") button_id = 2;
    else if (buttonText == "Good") button_id = 3;
    else if (buttonText == "Easy") button_id = 4;

    // Process the card response
    if (!currentDeckObj.processCardResponse(this->currentCard, button_id)) {
        Logger::error("Card response could not be updated", "Main");
    }

    proceedToNextCard();
}

void MainWindow::on_StatsButton_clicked(){
    DiscordManager::updatePresence("Viewing Stats", "", "stats");
    // Prevent multiple dialogs from being opened if the user clicks rapidly
    static bool isOpening = false;
    if (isOpening) return;
    isOpening = true;

    Logger::info("Opening Statistics Dialog", "Main");

    StatsDialog* dialog = new StatsDialog(this);
    connect(dialog, &QDialog::finished, this, [this]() {
        isOpening = false;
    });
    connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
    dialog->exec();
}

void MainWindow::on_EndStudyButton_clicked() {
    ui->study->setVisible(false);
    ui->EndStudyButton->setVisible(false);

    ui->studyFinished->setVisible(true);

    if(!currentDeckObj.endStudy()){
        this->statusBar()->showMessage("Error: Could not end studying session.");
        return;
    }

    this->currentDeckID.clear();
}

