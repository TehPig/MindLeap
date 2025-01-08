#include <iostream>
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

#include <Backend/Classes/User.hpp>
#include <Backend/Database/setup.hpp>

#include "Frontend/mainwindow.h"
#include "Frontend/addcarddialog.h"
#include "Frontend/confirmationdialog.h"
#include "Frontend/customdialog.h"
#include "Frontend/selectuser.h"
#include "Frontend/hoverabletablewidget.h"
#include "forms/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow()) {
    ui->setupUi(this);

    // Make sure unwanted elements are hidden
    if(!ui->scrollArea->isVisible()) ui->scrollArea->setVisible(true);
    ui->deckWidget->setVisible(false);
    ui->SetDescriptionButton->setVisible(false);
    ui->AddCardButton->setVisible(false);

    // Make the table widget hoverable
    auto *tableWidget = qobject_cast<HoverableTableWidget*>(ui->CardList);
    connect(tableWidget, &HoverableTableWidget::rowHovered, this, &MainWindow::onRowHovered);
    connect(tableWidget, &HoverableTableWidget::rowLeft, this, &MainWindow::onRowLeft);

    // Once DB is initialized, fetch the current user, load the decks and display them to user
    auto db = Database::getInstance("app_data.db");
    if(!db->getDB().isOpen()) db->initialize();

    QSqlQuery sqlQuery(db->getDB());

    // Look for existing users
    sqlQuery.prepare("SELECT id FROM SavedUser LIMIT 1");
    if(!sqlQuery.exec() || !sqlQuery.next()){
        // Look for saved users
        sqlQuery.prepare("SELECT id FROM Users LIMIT 1");

        // No users found on the DB
        if(!sqlQuery.exec() || !sqlQuery.next()){
            std::cout << "Attempting to create default user..." << std::endl;
            // Create Default user
            User user("n_Default");
            if(!user.create()){
                ui->statusbar->showMessage("Error: Default User could not be created.");
                return;
            }
            user.select();

            // Create Default deck
            Deck deck("n_Default");
            if(!deck.create()){
                ui->statusbar->showMessage("Error: Default Deck could not be created.");
                return;
            }

            setWindowTitle(user.getUsername() + " | MindLeap");

            // List the user's decks
            populateTableWidget(user.listDecks());
            return;
        }
    }
    User user(sqlQuery.value("id").toString());
    if(user.listDecks().empty()){
        // Create Default deck
        Deck deck("n_Default");
        if(!deck.create()){
            ui->statusbar->showMessage("Error: Default Deck could not be created.");
            return;
        }
    }

    setWindowTitle(user.getUsername() + " | MindLeap");

    // List the user's decks
    populateTableWidget(user.listDecks());
}

MainWindow::~MainWindow() { delete ui; }

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
        qDebug() << "Column" << i << "Width set to:" << width;

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

                        nameItem->setText(name);
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
    CustomDialog *dialog = new CustomDialog(this);
    dialog->setWindowTitleText("Create Deck");
    dialog->setMessageText("Enter Deck name:");

    if (dialog->exec() == QDialog::Accepted) {
        QString deckName = dialog->getEnteredText(); // Now safe to get entered text

        Deck deck("n_" + deckName);
        const bool status = deck.create();
        if(!status){
            this->statusBar()->showMessage("Error: Deck was not created.");
            delete dialog;
            return;
        }

        QTableWidget* list = this->get_tableWidget();
        int row = list->rowCount();
        list->insertRow(row);

        insertTableRow(deck, row, true);

        delete dialog;
        ui->CardList->viewport()->update();

        // Show hidden buttons
        ui->CreateDeckButton->setVisible(false);
        ui->SetDescriptionButton->setVisible(true);

        showDeckInfo(deck);
    }
}

void MainWindow::on_DecksButton_clicked() {
    if(!ui->scrollArea->isVisible()){
        ui->deckWidget->setVisible(false);
        ui->scrollArea->setVisible(true);

        // Change button visibility
        ui->CreateDeckButton->setVisible(true);
        ui->SetDescriptionButton->setVisible(false);
        ui->AddCardButton->setVisible(false);

        adjustTableColumnWidths();
    }
}

void MainWindow::on_SetDescriptionButton_clicked() {
    CustomDialog* dialog = new CustomDialog(this);
    dialog->setWindowTitleText("Set Deck Description");
    dialog->setMessageText("Enter description:");

    if(dialog->exec() == QDialog::Accepted){
        QString deckDescription = dialog->getEnteredText(); // Now safe to get entered text

        const QString id = ui->Name->property("deckID").toString();
        Deck deck(id);
        const bool status = deck.setDescription(deckDescription);
        if(!status){
            this->statusBar()->showMessage("Error: Deck Description was not set.");
            delete dialog;
            return;
        }

        if(!ui->scrollArea_2->isVisible()) ui->scrollArea_2->setVisible(true);

        ui->Description->setText(deckDescription);
        delete dialog;
    }
}

void MainWindow::on_AddCardButton_clicked() {
    AddCardDialog* dialog = new AddCardDialog(this);

    if(dialog->exec() == QDialog::Accepted){
        const QString id = ui->Name->property("deckID").toString();
        const QString question = dialog->getQuestion();
        const QString answer = dialog->getAnswer();

        Card card(question, answer);
        Deck deck(id);
        const bool status = deck.addCard(card);
        if(!status){
            this->statusBar()->showMessage("Error: Card not added to Deck");
            delete dialog;
            return;
        }

        showDeckInfo(deck);
        delete dialog;
    }
}

// Helper Methods
void MainWindow::showDeckInfo(const Deck& deck) {
    ui->CreateDeckButton->setVisible(false);
    ui->SetDescriptionButton->setVisible(true);
    ui->AddCardButton->setVisible(true);

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

    ui->UnseenCount->setText(QString::number(deck.getCardCount()));
    ui->PendingCount->setText("0");
    ui->ReviewCount->setText("0");

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

    connect(nameLabel, &QLabel::linkActivated, this, [this, deck]() {
        showDeckInfo(deck);
    });

    ui->CardList->setCellWidget(row, 0, nameLabel);

    QTableWidgetItem *nameItem = new QTableWidgetItem();
    nameItem->setData(Qt::UserRole, QVariant(deck.getID()));
    ui->CardList->setItem(row, 0, nameItem);

    // Set New, Learn, Review, and Total values
    auto *newItem = new QTableWidgetItem(!insert_default_values ? "1" : "0");
    newItem->setFont(font);
    newItem->setTextAlignment(Qt::AlignCenter);
    newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable); // Make non-editable
    ui->CardList->setItem(row, 1, newItem);

    auto *learnItem = new QTableWidgetItem(!insert_default_values ? "1" : "0");
    learnItem->setFont(font);
    learnItem->setTextAlignment(Qt::AlignCenter);
    learnItem->setFlags(learnItem->flags() & ~Qt::ItemIsEditable); // Make non-editable
    ui->CardList->setItem(row, 2, learnItem);

    auto *reviewItem = new QTableWidgetItem(!insert_default_values ? "1" : "0");
    reviewItem->setFont(font);
    reviewItem->setTextAlignment(Qt::AlignCenter);
    reviewItem->setFlags(reviewItem->flags() & ~Qt::ItemIsEditable); // Make non-editable
    ui->CardList->setItem(row, 3, reviewItem);

    auto *totalItem = new QTableWidgetItem(QString::number(deck.getCardCount()));
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

bool MainWindow::updateTableRow(const QString& id){
    for (int row = 0; row < ui->CardList->rowCount(); ++row) {
        QTableWidgetItem* item = ui->CardList->item(row, 0);
        if (item && item->data(Qt::UserRole).toString() == id){

        }
    }

    return false;
}

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
