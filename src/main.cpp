#include <iostream>
#include <QApplication>
#include <QSqlQuery>
#include "Backend/Classes/Card.hpp"
#include "Backend/Classes/Deck.hpp"
#include "Backend/Classes/User.hpp"
#include "Backend/Database/setup.hpp"
#include "Frontend/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    /*
    // Initialize the database instance
    if (Database* db = Database::getInstance("app_data.db")) {
        try {
            db->initialize();
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << "\n";
            return -1;
        }
    } else {
        std::cerr << "Failed to initialize the database instance.\n";
        return -1;
    }

    // Create some cards and decks for testing purposes
    const Card card("what is this", "a test");
    const Card card2("another question", "answer here");

    const std::vector<Card> cards = {card, card2};
    Deck test("test deck", cards);

    const std::vector<Deck> decks = {test};
    User user("n_test user");

    // Attempt to create the user in the database
    if (const bool user_created = user.create(); !user_created) {
        std::cerr << "Error creating user.\n";
    } else {
        std::cout << "User created successfully.\n";
    }

    // Add a new card to the deck
    Card newCard("new question", "new answer");
    if (newCard.createCard()) {
        std::cout << "Card added successfully.\n";
    } else {
        std::cerr << "Failed to add card.\n";
    }

    // List the decks and cards in the console
    std::cout << "==========================\n";
    for (const auto &deck : User::listDecks()) {
        std::cout << "Deck: " << deck.getName().toStdString() << "\n";
        for (const auto &cardd : deck.listCards()) {
            std::cout << "  Card: " << cardd.getQuestion().toStdString() << " - " << cardd.getAnswer().toStdString() << "\n";
        }
    }
    std::cout << "==========================\n";
    */

    // Create and display the MainWindow
    MainWindow mainWindow;
    mainWindow.show();

    // Enter the event loop
    return app.exec();
}