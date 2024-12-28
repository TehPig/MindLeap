#include <QSqlError>
#include <QSqlQuery>

#include <Backend/Classes/User.hpp>
#include <Backend/Database/setup.hpp>

#include <iostream>
#include <algorithm>

// Constructor
User::User(const QString &u, int i, const std::vector<Deck> &d)
    : username(u), id(i), decks(d), selected(nullptr), times_seen(0), last_viewed(0), next_review(0) {}

// Getters
void User::listDecks() const {
    for (size_t i = 0; i < this->decks.size(); ++i) {
        std::cout << i << ". " << decks[i].getName().toStdString()
        << " - " << decks[i].getID() << "\n";
    }
}

// Setters
void User::selectUser(int id) {
    // In a real-world application, we'd query the database to verify and load the user.
    if (this->id == id) {
        this->selected = this;
        std::cout << "Selected User: " << this->username.toStdString() << "\n";
    } else {
        std::cerr << "[User] User ID does not match this instance.\n";
    }
}

// Database Operations

// Create a new user in the database
bool User::createUser() const {
    Database *db = Database::getInstance();
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
    int user_id = query.lastInsertId().toInt();

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
bool User::renameUser(int id, QString newUsername) {
    Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("UPDATE Users SET username = ? WHERE id = ?;");
    query.addBindValue(newUsername);
    query.addBindValue(id);

    if (!query.exec()) {
        std::cerr << "[DB] Failed to rename user: " << query.lastError().text().toStdString() << "\n";
        return false;
    }

    // Update in-memory data
    if (this->id == id) this->username = newUsername;

    return true;
}

// Delete a user
bool User::deleteUser(int id) {
    Database *db = Database::getInstance();
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
    decks.push_back(deck);
}

// Remove a deck from the user's list
bool User::removeDeck(const Deck &deck) {
    auto it = std::find(this->decks.begin(), this->decks.end(), deck);
    if (it == this->decks.end()) return false; // Deck not found

    this->decks.erase(it);
    return true;
}
