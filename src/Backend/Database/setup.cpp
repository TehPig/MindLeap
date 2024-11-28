//
// Created by TehPig on 11/27/2024.
//
#include <sqlite3.h>
#include <Backend/Database/setup.hpp>

#include <iostream>
#include <string>

Database* Database::instance = nullptr;

Database::Database() : db(nullptr) {}

Database::Database(const std::string &path) : path(path), db(nullptr) {}
Database::~Database() {
    if (db) sqlite3_close(db);
}

sqlite3 *Database::getDB() const {
    return db;
}

void Database::initialize() {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << "\n";
        std::cout << "[Database] Could not open database.";
    }

    // Create tables
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL, times_seen INTEGER, last_viewed INTEGER, next_review INTEGER);", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Decks (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, user_id INTEGER NOT NULL, FOREIGN KEY(user_id) REFERENCES Users(id));", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Cards (id INTEGER PRIMARY KEY AUTOINCREMENT, question TEXT NOT NULL, answer TEXT NOT NULL, deck_id INTEGER NOT NULL, FOREIGN KEY(deck_id) REFERENCES Decks(id));", nullptr, nullptr, nullptr);

    std::cout << "[Database] Initialized successfully!\n";
}

bool Database::execute(const std::string &query) const {
    char *err;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "Error: " << err << "\n";
        sqlite3_free(err);
        return false;
    }

    return true;
}

Database* Database::getInstance(const std::string &path) {
    if (instance == nullptr) {
        instance = new Database(path);
    }
    return instance;
}