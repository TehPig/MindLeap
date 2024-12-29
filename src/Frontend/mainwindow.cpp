#include <iostream>
#include <Backend/Classes/User.hpp>
#include <Backend/Database/setup.hpp>
#include "Frontend/mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Once DB is initialized, fetch the current user, load the decks and display them to user
    Database* db = Database::getInstance("app_data.db");

    // Create a User object with default values
    User user("", 0, {});

    // Select the user by ID (replace 1 with the actual user ID you want to select)
    user.selectUser(1);

    // List the user's decks
    std::vector<Deck> decks = user.listDecks();
    populateListWidget(decks);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    std::cout << "Button clicked" << std::endl;
}

void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    // Define the behavior when the current row changes
    qDebug() << "Current row changed to:" << currentRow;
}

void MainWindow::populateListWidget(const std::vector<Deck> &decks) {
    for (size_t i = 0; i < decks.size(); ++i) {
        QString itemText = QString("%1. %2 (Total Cards: %3)").arg(i + 1).arg(decks[i].getName()).arg(decks[i].getCardCount());
        ui->listWidget->addItem(itemText);
    }
}