#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QString>

#include "Backend/Classes/User.hpp"
#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/generateID.hpp"
#include "Backend/Utilities/createUniqueUser.hpp"

// Constructors
//User::User(const QString& u, const QString& i, const std::vector<Deck> &d)
//    : username(u), id(i), decks(d) {}
User::User(const QString& name, const QString& id)
    : username(name), id(id) {}
User::User(const QString& name_or_id) {
    if (name_or_id.startsWith("n_")) this->username = name_or_id.trimmed();
    else this->id = name_or_id.trimmed();
}
// For the user of listUsers function
User::User() {}

// Getters
// Get username
QString User::getUsername() const {
    // If ID is set, retrieve the username from the database
    if (!this->id.isEmpty()) {
        const Database *db = Database::getInstance();
        QSqlQuery query(db->getDB());

        query.prepare(QStringLiteral("SELECT username FROM Users WHERE id = ?"));
        query.addBindValue(this->id);

        if (!query.exec() || !query.next()){
            qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
            return QString();
        }

        return query.value(0).toString();
    }
    return this->username.isEmpty() ? QString() : this->username;
}

// Get user ID
QString User::getID() const {
    return this->id.isEmpty() ? QString() : this->id;
}

// Setters


// Database Operations

// Create a new user in the database
bool User::create() {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    if (this->username.isEmpty()) {
        // Check if Default user exists
        query.prepare(QStringLiteral("SELECT COUNT(*) FROM Users WHERE username = 'Default';"));
        if (!query.exec()) {
            qDebug() << "[DB] Failed to check for default user: " << query.lastError().text();
            return false;
        }

        if (query.next() && query.value(0).toInt() > 0) {
            qDebug() << "[DB] Default user already exists. For a new user, specify a username.";
            return false;
        }

        return createUniqueUser("Default");
    }

    return createUniqueUser(this->username.mid(2));
}

// Delete a user
bool User::_delete() const {
    if (this->id.isEmpty()) {
        qDebug() << "[DB] User Delete - Missing ID.";
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("DELETE FROM Users WHERE id = ?;"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to delete user: " << query.lastError().text();
        return false;
    }

    return true; // UsersDecks and UserStats are cleaned up automatically.
}

// Fetch a user from the database
User User::fetch() const {
    if (this->id.isEmpty() && this->username.isEmpty()) {
        qDebug() << "[DB] User Fetch - Unable to fetch without ID or username.";
        return User();
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM Users WHERE id = ?;"));
    query.addBindValue(this->id);

    if (!query.exec() || !query.next()) {
        query.prepare(QStringLiteral("SELECT * FROM Users WHERE username = ?;"));
        query.addBindValue(this->username);

        if (!query.exec() || !query.next()) {
            qDebug() << "[DB] No user found with the given ID or username.";
            return User();
        }

        return User(query.value("username").toString(), query.value("id").toString());
    }

    return User(query.value("username").toString(), query.value("id").toString());
}

// Find the selected user
User User::fetchSelected() {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1;"));

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] No selected user found.";
        return User();
    }

    return User(query.value("id").toString());
}

// Select user
bool User::select() const {
    if (this->id.isEmpty()) {
        qDebug() << "[DB] User Select - Missing ID.";
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Prepare the SQL query to select the user
    query.prepare(QStringLiteral("SELECT username FROM Users WHERE id = ?;"));
    query.addBindValue(this->id);

    // Execute the query and check for errors
    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] No user found with the given ID.";
        return false;
    }

    const QString username = query.value(QStringLiteral("username")).toString();

    // Check if there is a saved user
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM SavedUser LIMIT 1"));

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Failed to save selected user ID: " << query.lastError().text();
        return false;
    }

    if (query.value(0).toInt() == 0) query.prepare(QStringLiteral("INSERT INTO SavedUser (id) VALUES (?);"));
    else query.prepare(QStringLiteral("UPDATE SavedUser SET id = ?;"));

    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to save selected user ID: " << query.lastError().text();
        return false;
    }

    qDebug() << "[DB] Selected User: " << username;
    return true;
}

// Rename a user
bool User::rename(const QString &username) {
    if (this->id.isEmpty()) {
        qDebug() << "[DB] User Rename - Missing ID.";
        return false;
    }
    if (username.trimmed().isEmpty()) {
        qDebug() << "[DB] User Rename - Invalid username specified.";
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("UPDATE Users SET username = ? WHERE id = ?;"));
    query.addBindValue(username);
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to rename user: " << query.lastError().text();
        return false;
    }

    // Update class data
    this->username = username;
    return true;
}

// List all user's decks
std::vector<Deck> User::listDecks() const {
    std::vector<Deck> decks;
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Retrieve all deck IDs for the user
    query.prepare(QStringLiteral("SELECT deck_id FROM UsersDecks WHERE user_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "Failed to retrieve deck IDs: " << query.lastError().text();
        return decks;
    }

    QStringList deckIds;
    while (query.next()) {
        deckIds << query.value(0).toString(); // Use column index 0 for deck_id
    }

    if (deckIds.isEmpty()) return decks; // No decks found

    // Retrieve all decks using a single query, quote deck IDs properly
    QStringList quotedDeckIds;
    for (const QString& deckId : deckIds) {
        quotedDeckIds << QStringLiteral("'") + deckId + QStringLiteral("'");  // Ensure each deck_id is wrapped in single quotes
    }

    query.prepare(QStringLiteral("SELECT id, name FROM Decks WHERE id IN (%1)").arg(quotedDeckIds.join(", ")));
    if (!query.exec()) {
        qDebug() << "Failed to retrieve decks: " << query.lastError().text();
        return decks;
    }

    while (query.next()) {
        QString deck_id = query.value("id").toString();
        QString deck_name = query.value("name").toString();

        decks.emplace_back(deck_name, deck_id);
    }

    return decks;
}

// List all users
std::vector<User> User::listUsers() {
    std::vector <User> users;
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM Users;"));
    if (!query.exec()) {
        qDebug() << "[DB] Failed to list users: " << query.lastError().text();
        return users;
    }

    while (query.next()){
        users.emplace_back(query.value("username").toString(), query.value("id").toString());
    }

    return users;
}

// Get User stats
void User::getStats() const {
    qDebug() << "User Stats";
}