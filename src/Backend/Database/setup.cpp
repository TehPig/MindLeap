//
// Created by TehPig on 11/27/2024.
//
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <Backend/Database/setup.hpp>

#include <iostream>
#include <string>

Database* Database::instance = nullptr;

Database::Database(const std::string &path) : path(path), db(QSqlDatabase::addDatabase("QSQLITE")) {
    db.setDatabaseName(QString::fromStdString(path));
}

Database::~Database() {
    if (db.isOpen()) db.close();
}

QSqlDatabase Database::getDB() const {
    return db;
}

void Database::initialize() {
    if (!db.open()) {
        std::cerr << "[Database] Could not open database: " << db.lastError().text().toStdString() << "\n";
        exit(EXIT_FAILURE);
    }

    // Create tables
    if (!execute("CREATE TABLE IF NOT EXISTS Users ("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                 "username TEXT NOT NULL UNIQUE, "
                 "times_seen INTEGER, "
                 "last_viewed INTEGER, "
                 "next_review INTEGER);") ||
        !execute("CREATE TABLE IF NOT EXISTS Decks ("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                 "name TEXT NOT NULL UNIQUE, "
                 "user_id INTEGER NOT NULL, "
                 "FOREIGN KEY(user_id) REFERENCES Users(id));") ||
        !execute("CREATE TABLE IF NOT EXISTS Cards ("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                 "question TEXT NOT NULL, "
                 "answer TEXT NOT NULL, "
                 "deck_id INTEGER NOT NULL, "
                 "FOREIGN KEY(deck_id) REFERENCES Decks(id));") ||
        !execute("CREATE TABLE IF NOT EXISTS DecksCards ("
                 "deck_id INTEGER NOT NULL, "
                 "card_id INTEGER NOT NULL, "
                 "PRIMARY KEY(deck_id, card_id), "
                 "FOREIGN KEY(deck_id) REFERENCES Decks(id) ON DELETE CASCADE, "
                 "FOREIGN KEY(card_id) REFERENCES Cards(id) ON DELETE CASCADE);")) {
        std::cerr << "[Database] Failed to initialize database.\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "[Database] Initialized successfully!\n";
}

bool Database::prepare(const QString &query, const QVariantList &params) const {
    QSqlQuery sqlQuery(db);
    if (!sqlQuery.prepare(query)) {
        std::cerr << "[DB] Query preparation failed: " << query.toStdString() << "\n";
        std::cerr << "[DB] Error: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    // Bind parameters
    for (const auto &param : params) {
        sqlQuery.addBindValue(param);
    }

    // Execute the query
    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Query execution failed: " << query.toStdString() << "\n";
        std::cerr << "[DB] Error: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    return true;
}

bool Database::execute(const QString &query) const {
    QSqlQuery sqlQuery(db);
    if (!sqlQuery.exec(query)) {
        std::cerr << "[DB] Query execution failed: " << query.toStdString() << "\n";
        std::cerr << "[DB] Error: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    return true;
}

Database* Database::getInstance(const std::string &path) {
    if (instance == nullptr) instance = new Database(path);
    return instance;
}
