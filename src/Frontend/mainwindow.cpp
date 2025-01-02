#include <iostream>

#include <QSqlQuery>
#include <QTableWidget>
#include <QIcon>
#include <QTimer>
#include <QSize>

#include <Backend/Classes/User.hpp>
#include <Backend/Database/setup.hpp>

#include "Frontend/mainwindow.h"
#include "Frontend/createdeckdialog.h"
#include "Frontend/selectuser.h"
#include <Frontend/hoverabletablewidget.h>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->deckWidget->setVisible(false);

    // Make the table widget hoverable
    HoverableTableWidget *tableWidget = qobject_cast<HoverableTableWidget*>(ui->tableWidget);
    connect(tableWidget, &HoverableTableWidget::rowHovered, this, &MainWindow::onRowHovered);
    connect(tableWidget, &HoverableTableWidget::rowLeft, this, &MainWindow::onRowLeft);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // Once DB is initialized, fetch the current user, load the decks and display them to user
    Database* db = Database::getInstance("app_data.db");
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
            const bool user_created = user.create();
            if(!user_created){
                ui->statusbar->showMessage("Error: Default User could not be created.");
                return;
            }
            user.select();

            // Create Default deck
            Deck deck("n_Default");
            const bool deck_created = deck.create();
            if(!deck_created){
                ui->statusbar->showMessage("Error: Default Deck could not be created.");
                return;
            }

            setWindowTitle(user.getUsername() + " | MindLeap");

            // List the user's decks
            std::vector<Deck> decks = user.listDecks();
            populateTableWidget(decks);

            return;
        }
    }
    User user(sqlQuery.value("id").toString());
    if(user.listDecks().size() == 0){
        // Create Default deck
        Deck deck("n_Default");
        const bool deck_created = deck.create();
        if(!deck_created){
            ui->statusbar->showMessage("Error: Default Deck could not be created.");
            return;
        }
    }

    setWindowTitle(user.getUsername() + " | MindLeap");

    // List the user's decks
    std::vector<Deck> decks = user.listDecks();
    populateTableWidget(decks);
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

    std::cout << "Button clicked" << std::endl;
    std::cout << "State: " << ui->tableWidget->isVisible() << std::endl;
}

/*void MainWindow::on_listWidget_currentRowChanged(int currentRow) {
    // Define the behavior when the current row changes
    qDebug() << "Current row changed to:" << currentRow;
}*/

void MainWindow::populateTableWidget(const std::vector<Deck> &decks) {
    ui->tableWidget->setRowCount(decks.size());
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget->viewport()->setAttribute(Qt::WA_Hover, false);
    ui->tableWidget->viewport()->setFocusPolicy(Qt::NoFocus);

    for (int i = 0; i < decks.size(); i++) {
        const Deck &deck = decks[i];

        // Set Names
        QTableWidgetItem *nameItem = new QTableWidgetItem(deck.getName());
        nameItem->setData(Qt::UserRole, QVariant(deck.getID()));
        ui->tableWidget->setItem(i, 0, nameItem);

        // Set Total
        QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(deck.getCardCount()));
        totalItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(i, 4, totalItem);

        // Settings Button in Container
        QPushButton *settingsButton = new QPushButton();
        settingsButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_CommandLink));
        settingsButton->setIconSize(QSize(64, 64));
        settingsButton->setStyleSheet(R"(QPushButton {
                                            border: none;
                                            background: transparent;
                                            color: #a9b7c6;
                                        }
                                        QPushButton:hover {
                                            background: rgba(0, 123, 80, 0.2);
                                        }
                                        QPushButton:pressed {
                                            background: rgba(0, 123, 80, 0.3);
                                        }
                                        QPushButton:focus {
                                            outline: none;
                                        })");

        settingsButton->setFocusPolicy(Qt::NoFocus); // Avoid focus interference
        // settingsButton->setAttribute(Qt::WA_TransparentForMouseEvents, false); // Ensure mouse events are captured
        settingsButton->installEventFilter(this);

        QWidget *container = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0); // Remove margins
        layout->addWidget(settingsButton);
        layout->setAlignment(Qt::AlignCenter);
        container->setLayout(layout);

        ui->tableWidget->setCellWidget(i, 5, container);

        // Delay hiding the widget after event processing
        QTimer::singleShot(0, container, [container]() {
            container->setVisible(false);
        });
    }

    // Force refresh
    //ui->tableWidget->viewport()->update();
    //ui->tableWidget->repaint();
}

void MainWindow::onRowHovered(int row) {
    static int lastHoveredRow = -1; // Static to remember previous row

    // Avoid unnecessary updates
    if (lastHoveredRow == row) return;

    // Hide the button from the previously hovered row
    if (lastHoveredRow != -1) {
        QWidget* previousWidget = ui->tableWidget->cellWidget(lastHoveredRow, 5);
        //if (QPushButton* button = qobject_cast<QPushButton*>(previousWidget)) {
            previousWidget->setVisible(false);
        //}
    }

    // Show the button for the newly hovered row
    if (row != -1) {
        QWidget* currentWidget = ui->tableWidget->cellWidget(row, 5);
        //if (QPushButton* button = qobject_cast<QPushButton*>(currentWidget)) {
            currentWidget->setVisible(true);
        //}
    }

    lastHoveredRow = row; // Update last hovered row
}

void MainWindow::onRowLeft(int row) {
    QWidget* widget = ui->tableWidget->cellWidget(row, 5);
    //if (QPushButton* button = qobject_cast<QPushButton*>(widget)) {
        widget->setVisible(false);
    //}
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPushButton *button = qobject_cast<QPushButton *>(obj);

        if (button && mouseEvent->button() == Qt::LeftButton) {
            showDeckSettings(button); // Call your context menu function
            return true; // Event handled
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::showDeckSettings(QPushButton *button) {
    QMenu menu;
    menu.addAction("Option 1", this, []() { qDebug() << "Option 1 selected"; });
    menu.addAction("Option 2", this, []() { qDebug() << "Option 2 selected"; });

    menu.setStyleSheet(R"(
        QMenu {
            background-color: #1e1d23;
            color: #a9b7c6;
            border: 1px solid #444;
        }
        QMenu::item {
            padding: 4px 8px;
        }
        QMenu::item:selected {
            background-color: #007b50;
            color: #FFFFFF;
        }
    )");

    QPoint globalPos = button->mapToGlobal(QPoint(0, button->height()));
    menu.exec(globalPos);

    // Force button to repaint after menu closes
    button->update();
}

// void MainWindow::set_active_widget(const std::string widget){
//     if(widget == "list" && ui->listWidget->isHidden()) ui->listWidget->show();
//     if(widget == "card_info" && ui->widget_2->isHidden()) ui->widget_2->show();

//     if(widget == "list"){
//         this->
// }

void MainWindow::on_pushButton_5_clicked() {
    CreateDeckDialog* dialog = new CreateDeckDialog(this);
    dialog->exec();

    dialog->setAttribute(Qt::WA_DeleteOnClose); // Delete the dialog on close
}

void MainWindow::on_pushButton_6_clicked() {
    ui->deckWidget->isHidden() ? ui->deckWidget->show() : ui->deckWidget->hide();
}

// Action Buttons
void MainWindow::on_actionUsers_triggered() {
    this->close();

    selectuser *selectUser = new selectuser();
    selectUser->show();

    selectUser->setAttribute(Qt::WA_DeleteOnClose); // Delete select user window on close
}

void MainWindow::on_actionFullscreen_triggered() { this->isFullScreen() ? this->showNormal() : this->showFullScreen() ; }


