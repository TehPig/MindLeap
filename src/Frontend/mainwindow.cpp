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
#include "Frontend/confirmationdialog.h"
#include "Frontend/customdialog.h"
#include "Frontend/selectuser.h"
#include <Frontend/hoverabletablewidget.h>
#include "forms/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow()) {
    ui->setupUi(this);

    ui->deckWidget->setVisible(false);

    // Make the table widget hoverable
    auto *tableWidget = qobject_cast<HoverableTableWidget*>(ui->tableWidget);
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

    QTimer::singleShot(0, this, [this]() {
        adjustTableColumnWidths();
    });
}

MainWindow::~MainWindow() { delete ui; }

HoverableTableWidget* MainWindow::get_tableWidget() {
    return static_cast<HoverableTableWidget*>(ui->tableWidget);
}

void MainWindow::on_pushButton_clicked() {
    if(!ui->tableWidget->isVisible()){
        ui->deckWidget->setVisible(false);
        ui->tableWidget->setVisible(true);
    }
}

void MainWindow::populateTableWidget(const std::vector<Deck>& decks) {
    ui->tableWidget->setRowCount(decks.size());
    QFont font;
    font.setPointSize(12);

    for (int i = 0; i < decks.size(); i++) {
        const Deck &deck = decks[i];

        // Set Names
        auto *nameItem = new QTableWidgetItem(deck.getName());
        nameItem->setTextAlignment(Qt::AlignLeft);

        QFont boldFont = nameItem->font();
        boldFont.setBold(true);
        boldFont.setPointSize(11);
        nameItem->setFont(boldFont); // Apply bold font

        nameItem->setData(Qt::UserRole, QVariant(deck.getID()));
        ui->tableWidget->setItem(i, 0, nameItem);

        // Set New, Learn, Review, and Total values
        auto *newItem = new QTableWidgetItem("1"); // Example values
        newItem->setTextAlignment(Qt::AlignCenter);
        newItem->setFont(font);
        ui->tableWidget->setItem(i, 1, newItem);

        auto *learnItem = new QTableWidgetItem("2"); // Example values
        learnItem->setTextAlignment(Qt::AlignCenter);
        learnItem->setFont(font);
        ui->tableWidget->setItem(i, 2, learnItem);

        auto *reviewItem = new QTableWidgetItem("3"); // Example values
        reviewItem->setTextAlignment(Qt::AlignCenter);
        reviewItem->setFont(font);
        ui->tableWidget->setItem(i, 3, reviewItem);

        auto *totalItem = new QTableWidgetItem(QString::number(deck.getCardCount())); // Example values
        totalItem->setTextAlignment(Qt::AlignCenter);
        totalItem->setFont(font);
        ui->tableWidget->setItem(i, 4, totalItem);

        // Settings Button in Container (Hidden by default)
        auto *settingsButton = new QPushButton();
        settingsButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_CommandLink));
        settingsButton->setIconSize(QSize(32, 32));  // Smaller icon size for better fit
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
        settingsButton->setVisible(false);  // Initially hide the button

        // Connect the settings button to the context menu or other actions
        connect(settingsButton, &QPushButton::clicked, this, [this, deck, settingsButton]() {
            // Example of handling the click on the settings button
            //keepButtonVisible();
            QTableWidget* tableWidget = ui->tableWidget;

            // Find the row containing this button
            for (int row = 0; row < tableWidget->rowCount(); ++row) {
                if (tableWidget->cellWidget(row, 5) == settingsButton->parentWidget()) {
                    qDebug() << "Settings button clicked for row" << row;

                    showDeckSettings(deck, row);
                    return;
                }
            }
        });

        // Container for the button in column 5
        auto *container = new QWidget();
        auto *layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0); // Remove margins
        layout->addWidget(settingsButton);
        layout->setAlignment(Qt::AlignCenter);
        container->setLayout(layout);

        ui->tableWidget->setCellWidget(i, 5, container);

        // Install event filter to detect hover and hide/show the button
        container->installEventFilter(this);
    }
}

void MainWindow::adjustTableColumnWidths() {
    QTableWidget* table = ui->tableWidget;

    // Total available width
    double totalWidth = static_cast<double>(table->viewport()->width());

    // Subtract the width of the vertical scrollbar if it is visible
    if (table->verticalScrollBar()->isVisible()) {
        totalWidth -= static_cast<double>(table->verticalScrollBar()->width());
    }

    // Proportions for each column except the last one
    const double proportions[] = {0.45, 0.1, 0.1, 0.1, 0.1};
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
    if(!ui->tableWidget->isVisible()) return;
    QMainWindow::resizeEvent(event); // Call base class implementation
    adjustTableColumnWidths();       // Adjust column widths dynamically
}

void MainWindow::setButtonVisibility(int row, bool visible) {
    auto *container = ui->tableWidget->cellWidget(row, 5);
    if (container && !ui->tableWidget->getContextMenuActive()) {
        container->findChild<QPushButton *>()->setVisible(visible);
    }
}

// Context menu function that opens on settings button click
void MainWindow::showDeckSettings(const Deck& deck, const int row) {
    auto *tableWidget = qobject_cast<HoverableTableWidget*>(ui->tableWidget);
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
                    if (container) {
                        container->findChild<QPushButton *>()->setVisible(false);
                    }

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
            if (container) {
                container->findChild<QPushButton *>()->setVisible(false);
            }
        }
        tableWidget->setContextMenuActive(false);
    });

    // Show the context menu
    contextMenu.exec(QCursor::pos());
}

void MainWindow::onRowHovered(int row) { setButtonVisibility(row, true); }

void MainWindow::onRowLeft(int row) { setButtonVisibility(row, false); }

// Create Deck dialog
void MainWindow::on_pushButton_5_clicked() {
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
        list->insertRow(row); // Insert a new row

        // Set Names
        auto *nameItem = new QTableWidgetItem(deck.getName());

        QFont boldFont = nameItem->font();
        boldFont.setBold(true);
        nameItem->setFont(boldFont); // Apply bold font

        nameItem->setData(Qt::UserRole, QVariant(deck.getID()));
        ui->tableWidget->setItem(row, 0, nameItem);

        // Set New, Learn, Review, and Total values
        auto *newItem = new QTableWidgetItem("0"); // Example values
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, 1, newItem);

        auto *learnItem = new QTableWidgetItem("0"); // Example values
        learnItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, 2, learnItem);

        auto *reviewItem = new QTableWidgetItem("0"); // Example values
        reviewItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, 3, reviewItem);

        auto *totalItem = new QTableWidgetItem(QString::number(deck.getCardCount())); // Example values
        totalItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, 4, totalItem);

        // Settings Button in Container (Hidden by default)
        auto *settingsButton = new QPushButton();
        settingsButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_CommandLink));
        settingsButton->setIconSize(QSize(32, 32));  // Smaller icon size for better fit
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
        settingsButton->setVisible(false);  // Initially hide the button

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

        list->setCellWidget(row, 5, container);

        // Install event filter to detect hover and hide/show the button
        container->installEventFilter(this);

        delete dialog;
        list->viewport()->update();

        list->setVisible(false);
        ui->widget_2->setVisible(true);

        // Ensure the QLabel widgets are properly initialized
        if (ui->Name && ui->CardCount) {
            // Set the text color and background color
            ui->Name->setStyleSheet("color: red; background-color: white;");
            ui->CardCount->setStyleSheet("color: red; background-color: white;");

            // Ensure the QLabel widgets are visible
            ui->Name->setVisible(true);
            ui->CardCount->setVisible(true);

            // Set the text
            ui->Name->setText(deck.getName());
            ui->CardCount->setText("Total Cards: " + QString::number(deck.getCardCount()));
            ui->Name->setStyleSheet("color: red; background-color: white;");
            ui->CardCount->setStyleSheet("color: red; background-color: white;");

            ui->Name->adjustSize();
            ui->CardCount->adjustSize();
        } else {
            qDebug() << "Error: QLabel widgets are not properly initialized.";
        }
    }
}

// Debug button
void MainWindow::on_pushButton_6_clicked() {
    ui->deckWidget->isHidden() ? ui->deckWidget->show() : ui->deckWidget->hide();
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
