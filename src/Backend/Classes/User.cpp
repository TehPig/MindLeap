#include <iostream>
#include <algorithm>

#include <QSqlError>
#include <QSqlQuery>

#include <Backend/Classes/User.hpp>
#include <Backend/Database/setup.hpp>
#include <utility>

#include "Backend/Utilities/generateID.hpp"

// Constructor
//User::User(const QString& u, const QString& i, const std::vector<Deck> &d)
//    : username(u), id(i), decks(d) {}
User::User(const QString& name, const QString& id)
    : username(name), id(id) {}
User::User(const QString& name_or_id) {
    if (name_or_id.startsWith("n_")) this->username = name_or_id;
    else this->id = name_or_id;
}
// For the user of listUsers function
User::User() {}

// Getters
QString User::getUsername() const {
    // If ID is set, retrieve the username from the database
    if (this->id.data()) {
        const Database *db = Database::getInstance();
        QSqlQuery sqlQuery(db->getDB());

        sqlQuery.prepare("SELECT username FROM Users WHERE id = ?");
        sqlQuery.addBindValue(this->id);

        if (!sqlQuery.exec() || !sqlQuery.next()){
            std::cerr << "[DB] Failed to execute query: " << sqlQuery.lastError().text().toStdString() << std::endl;
            return "username_u_404";
        }

        return sqlQuery.value(0).toString();
    }
    if(!this->username.data()) return "username_u_404";
    return this->username;
}

QString User::getID() const {
    if (!this->id.data()) return "ID_u_4o4";
    return this->id;
}

// Lists user's decks
std::vector<Deck> User::listDecks() {
    std::vector<Deck> decks;
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Step 1: Retrieve all deck IDs for the user
    query.prepare("SELECT deck_id FROM UsersDecks WHERE user_id = ?");
    query.addBindValue(this->id);

    if (!query.exec()) {
        std::cerr << "Failed to retrieve deck IDs: " << query.lastError().text().toStdString() << std::endl;
        return decks;
    }

    QStringList deckIds;
    while (query.next()) {
        deckIds << query.value(0).toString(); // Use column index 0 for deck_id
    }

    if (deckIds.isEmpty()) return decks; // No decks found

    // Step 2: Retrieve all decks using a single query, quote deck IDs properly
    QStringList quotedDeckIds;
    for (const QString& deckId : deckIds) {
        quotedDeckIds << "'" + deckId + "'";  // Ensure each deck_id is wrapped in single quotes
    }

    QString queryString = "SELECT id, name FROM Decks WHERE id IN (" + quotedDeckIds.join(",") + ")";

    query.prepare(queryString);
    if (!query.exec()) {
        std::cerr << "Failed to retrieve decks: " << query.lastError().text().toStdString() << std::endl;
        return decks;
    }

    while (query.next()) {
        QString deck_id = query.value(0).toString();  // Use column index 0 for id
        QString deck_name = query.value(1).toString();  // Use column index 1 for name

        decks.emplace_back(deck_name, deck_id);
    }

    return decks;
}

// Setters

// Select user
bool User::select() {
    if (!this->id.data()) {
        std::cerr << "[DB] User Select - Missing ID." << std::endl;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Prepare the SQL query to select the user
    query.prepare("SELECT username FROM Users WHERE id = ?;");
    query.addBindValue(this->id);

    // Execute the query and check for errors
    if (!query.exec() || !query.next()) {
        std::cerr << "[DB] No user found with the given ID." << std::endl;
        return false;
    }

    const QString username = query.value("username").toString();

    // Check if there is a saved user
    query.prepare("SELECT COUNT(*) FROM SavedUser LIMIT 1");

    if (!query.exec() || !query.next()) {
        std::cerr << "[DB] Failed to save selected user ID: " << query.lastError().text().toStdString() << std::endl;
        return false;
    }

    int count = query.value(0).toInt();
    if (count == 0) query.prepare("INSERT INTO SavedUser (id) VALUES (?);");
    else query.prepare("UPDATE SavedUser SET id = ?;");

    query.addBindValue(this->id);

    if (!query.exec()) {
        std::cerr << "[DB] Failed to save selected user ID: " << query.lastError().text().toStdString() << std::endl;
        return false;
    }

    std::cout << "[DB] Selected User: " << username.toStdString() << std::endl;
    return true;
}

// Database Operations

// Create a new user in the database
bool User::create() {
    if (!this->username.data()) {
        std::cerr << "[DB] User Create - Missing username." << std::endl;
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    std::string id = generateID();
    bool idExists;

    do {
        // Generate a unique ID
        id = generateID();
        std::cout << "[Debug - User] Generated ID: " << id << " for name " << this->username.mid(2).toStdString() << std::endl;

        // Insert the user into the Users table
        query.prepare("INSERT INTO Users (id, username) VALUES (?, ?);");
        query.addBindValue(QString::fromStdString(id));
        query.addBindValue(this->username.mid(2));

        if (!query.exec()) {
            if (query.lastError().nativeErrorCode() == "19") { // SQLite constraint violation error code
                std::cerr << "[DB] ID already exists, generating a new one." << std::endl;
                idExists = true;
            } else {
                std::cerr << "[DB] Failed to create user: " << query.lastError().text().toStdString() << std::endl;
                return false;
            }
        } else {
            idExists = false;
        }
    } while (idExists);

    std::cout << "User created successfully with ID: " << id << std::endl;

    this->id = QString::fromStdString(id);
    return true;
}

// Rename a user
bool User::rename(const QString &username) {
    if (!this->id.data()) {
        std::cerr << "[DB] User Rename - Missing ID." << std::endl;
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("UPDATE Users SET username = ? WHERE id = ?;");
    query.addBindValue(username);
    query.addBindValue(this->id);

    if (!query.exec()) {
        std::cerr << "[DB] Failed to rename user: " << query.lastError().text().toStdString() << std::endl;
        return false;
    }

    // Update in-memory data
    this->username = username;
    return true;
}

// Delete a user
bool User::_delete() const
{
    if (!this->id.data()) {
        std::cerr << "[DB] User Delete - Missing ID." << std::endl;
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("DELETE FROM Users WHERE id = ?;");
    query.addBindValue(this->id);

    if (!query.exec()) {
        std::cerr << "[DB] Failed to delete user: " << query.lastError().text().toStdString() << std::endl;
        return false;
    }

    return true;
}

// List all users
std::vector<User> User::listUsers() {
    std::vector <User> users;
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("SELECT * FROM Users;");
    if (!query.exec()) {
        std::cerr << "[DB] Failed to list users: " << query.lastError().text().toStdString() << std::endl;
        return users;
    }

    while (query.next()){
        QString id = query.value("id").toString();
        QString username = query.value("username").toString();
        users.emplace_back(username, id);
    }

    return users;
}

// Add a new deck to the user's list
void User::addDeck(const Deck &deck) {
    if (!this->id.data()) {
        std::cerr << "[DB] User Add Deck - Missing ID." << std::endl;
        return;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Prepare the SQL query to insert the new deck
    query.prepare("INSERT INTO Decks (name, user_id) VALUES (?, ?);");
    query.addBindValue(deck.getName());
    query.addBindValue(this->id);

    // Execute the query and check for errors
    if (!query.exec()) {
        std::cerr << "[DB] Failed to add deck: " << query.lastError().text().toStdString() << std::endl;
        return;
    }

    // Add the deck to the user's list of decks
    this->decks.push_back(deck);
}

// Remove a deck from the user's list
bool User::removeDeck(const Deck &deck) {
    if (!this->id.data()) {
        std::cerr << "[DB] User Remove Deck - Missing ID." << std::endl;
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Prepare the SQL query to delete the deck
    query.prepare("DELETE FROM Decks WHERE id = ? AND user_id = ?;");
    query.addBindValue(deck.getID());
    query.addBindValue(this->id);

    // Execute the query and check for errors
    if (!query.exec()) {
        std::cerr << "[DB] Failed to remove deck: " << query.lastError().text().toStdString() << std::endl;
        return false;
    }

    // Integrate Deck removal with the database properly

    return true;
}
