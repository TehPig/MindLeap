#include <iostream>
#include <algorithm>

#include <QSqlError>
#include <QSqlQuery>

#include <Backend/Classes/User.hpp>
#include <Backend/Database/setup.hpp>
#include <utility>

// Constructor
User::User(QString u, const int i, const std::vector<Deck> &d)
    : username(u), id(i), decks(d), selected(nullptr), times_seen(0), last_viewed(0), next_review(0) {}

// Getters

// User user's decks
std::vector<Deck> User::listDecks() {
    std::vector<Deck> decks;
    QSqlQuery query(Database::getInstance()->getDB());

    if (!query.exec("SELECT id, name FROM Decks")) {
        qDebug() << "Failed to retrieve decks:" << query.lastError().text();
        return decks;
    }

    while (query.next()) {
        int deck_id = query.value("id").toInt();
        QString deck_name = query.value("name").toString();
        decks.emplace_back(deck_name, deck_id);
    }

    return decks;
}

// Setters

// Select user
void User::selectUser(const int id) {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Prepare the SQL query to select the user
    query.prepare("SELECT username, times_seen, last_viewed, next_review FROM Users WHERE id = ?;");
    query.addBindValue(id);

    // Execute the query and check for errors
    if (!query.exec() || !query.next()) {
        std::cerr << "[DB] Failed to select user: " << query.lastError().text().toStdString() << "\n";
        return;
    }

    // Load user details
    this->id = id;
    this->username = query.value("username").toString();
    this->times_seen = query.value("times_seen").toInt();
    this->last_viewed = query.value("last_viewed").toInt();
    this->next_review = query.value("next_review").toInt();

    // Set the selected pointer to this instance
    this->selected = this;

    // Save the selected user ID to the database
    QSqlQuery saveQuery(db->getDB());
    saveQuery.prepare("UPDATE SavedUser SET user_id = ?;");
    saveQuery.addBindValue(id);

    if (!saveQuery.exec()) {
        std::cerr << "[DB] Failed to save selected user ID: " << saveQuery.lastError().text().toStdString() << "\n";
    }

    std::cout << "Selected User: " << this->username.toStdString() << "\n";
}

// Database Operations

// Create a new user in the database
bool User::createUser() const {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Insert the user into the Users table
    query.prepare("INSERT INTO Users (username, times_seen, last_viewed, next_review) VALUES (?, ?, ?, ?);");
    query.addBindValue(this->username);
    query.addBindValue(this->times_seen);
    query.addBindValue(this->last_viewed);
    query.addBindValue(this->next_review);

    if (!query.exec()) {
        std::cerr << "[DB] Failed to create user: " << query.lastError().text().toStdString() << "\n";
        return false;
    }

    // Retrieve the new user's ID
    const int user_id = query.lastInsertId().toInt();

    // Insert any decks associated with this user
    for (const auto &deck : this->decks) {
        QSqlQuery deckQuery(db->getDB());
        deckQuery.prepare("INSERT INTO Decks (name, user_id) VALUES (?, ?);");
        deckQuery.addBindValue(deck.getName());
        deckQuery.addBindValue(user_id);

        if (!deckQuery.exec()) {
            std::cerr << "[DB] Failed to create deck: " << deckQuery.lastError().text().toStdString() << "\n";
            return false;
        }
    }

    return true;
}

// Rename a user
bool User::renameUser(const int id, const QString &username) {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("UPDATE Users SET username = ? WHERE id = ?;");
    query.addBindValue(username);
    query.addBindValue(id);

    if (!query.exec()) {
        std::cerr << "[DB] Failed to rename user: " << query.lastError().text().toStdString() << "\n";
        return false;
    }

    // Update in-memory data
    if (this->id == id) this->username = username;

    return true;
}

// Delete a user
bool User::deleteUser(const int id) {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("DELETE FROM Users WHERE id = ?;");
    query.addBindValue(id);

    if (!query.exec()) {
        std::cerr << "[DB] Failed to delete user: " << query.lastError().text().toStdString() << "\n";
        return false;
    }

    return true;
}

// Add a new deck to the user's list
void User::addDeck(const Deck &deck) {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Prepare the SQL query to insert the new deck
    query.prepare("INSERT INTO Decks (name, user_id) VALUES (?, ?);");
    query.addBindValue(deck.getName());
    query.addBindValue(this->id);

    // Execute the query and check for errors
    if (!query.exec()) {
        std::cerr << "[DB] Failed to add deck: " << query.lastError().text().toStdString() << "\n";
        return;
    }

    // Add the deck to the user's list of decks
    this->decks.push_back(deck);
}

// Remove a deck from the user's list
bool User::removeDeck(const Deck &deck) {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Prepare the SQL query to delete the deck
    query.prepare("DELETE FROM Decks WHERE id = ? AND user_id = ?;");
    query.addBindValue(deck.getID());
    query.addBindValue(this->id);

    // Execute the query and check for errors
    if (!query.exec()) {
        std::cerr << "[DB] Failed to remove deck: " << query.lastError().text().toStdString() << "\n";
        return false;
    }

    // Remove the deck from the user's list of decks
    if (const auto it = std::find(this->decks.begin(), this->decks.end(), deck); it != this->decks.end()) {
        this->decks.erase(it);
    }

    return true;
}