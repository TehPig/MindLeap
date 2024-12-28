#include <QApplication> // QApplication for GUI applications
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <iostream>
#include "Backend/Classes/Card.hpp"
#include "Backend/Classes/Deck.hpp"
#include "Backend/Classes/User.hpp"
#include "Backend/Database/setup.hpp"
#include "Frontend/mainwindow.h" // Ensure this matches your file structure

int main(int argc, char *argv[]) {
    // Initialize the QApplication to enable GUI functionality
    QApplication app(argc, argv);

    // Debug message
    std::cout << "Initializing application..." << std::endl;

    // Initialize the database instance
    Database* db = Database::getInstance("app_data.db");
    if (db) {
        db->initialize();
    } else {
        std::cerr << "Failed to initialize the database instance.\n";
        return -1;
    }

    // Debug message
    std::cout << "Database initialized successfully." << std::endl;

    // Create some cards and decks for testing purposes
    const Card card("test card", 25, "what is this", "a test");
    const Card card2("test 2", 76, "another question", "answer here");

    const std::vector<Card> cards = {card, card2};
    const Deck test("test deck", 1234, cards);

    const std::vector<Deck> decks = {test};
    const User user("test user", 1234, decks);

    // Attempt to create the user in the database
    bool user_created = user.createUser();
    if (!user_created) {
        std::cerr << "Error creating user.\n";
    } else {
        std::cout << "User created successfully.\n";
    }

    // List the decks and cards in the console
    user.listDecks();
    std::cout << "==========================\n";
    test.listCards();
    std::cout << "==========================\n";
    std::cout << card.getName().toStdString() << " " << card.getID() << "\n";

    // Create and display the MainWindow
    MainWindow mainWindow;
    mainWindow.show();

    // Enter the event loop
    return app.exec();
}
