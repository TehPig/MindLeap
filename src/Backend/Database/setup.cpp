//
// Created by TehPig on 11/27/2024.
//
#include <sqlite3.h>
#include <Backend/Database/setup.hpp>

#include <iostream>
#include <string>
#include <vector>

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
        exit(EXIT_FAILURE);
    }

    // Create tables
    if(!execute("CREATE TABLE IF NOT EXISTS Users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL UNIQUE, times_seen INTEGER, last_viewed INTEGER, next_review INTEGER);") ||
    !execute("CREATE TABLE IF NOT EXISTS Decks (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL UNIQUE, user_id INTEGER NOT NULL, FOREIGN KEY(user_id) REFERENCES Users(id));") ||
    !execute("CREATE TABLE IF NOT EXISTS Cards (id INTEGER PRIMARY KEY AUTOINCREMENT, question TEXT NOT NULL, answer TEXT NOT NULL, deck_id INTEGER NOT NULL, FOREIGN KEY(deck_id) REFERENCES Decks(id));") ||
    !execute("CREATE TABLE IF NOT EXISTS DecksCards (deck_id INTEGER NOT NULL, card_id INTEGER NOT NULL, PRIMARY KEY(deck_id, card_id), FOREIGN KEY(deck_id) REFERENCES Decks(id) ON DELETE CASCADE, FOREIGN KEY(card_id) REFERENCES Cards(id) ON DELETE CASCADE);")) exit(EXIT_FAILURE);

    std::cout << "[Database] Initialized successfully!\n";
}

bool Database::prepare(const std::string &query, const std::vector<std::string> &stringParams, const std::vector<int> &intParams) const {
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DB] Query failed: " << query << "\n";
        std::cerr << "[DB] Error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    for(size_t i = 0; i < stringParams.size(); ++i){
        sqlite3_bind_text(stmt, i + 1, stringParams[i].c_str(), -1, SQLITE_STATIC);
    }

    for(size_t i = 0; i < intParams.size(); ++i){
        sqlite3_bind_int(stmt, stringParams.size() + i + 1, intParams[i]);
    }

    if(sqlite3_step(stmt) != SQLITE_DONE){
        std::cerr << "[DB] Query failed: " << query << "\n";
        std::cerr << "[DB] Error: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    return true;
}

bool Database::execute(const std::string &query) const {
    char *err;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "[DB] Query failed: " << query << "\n";
        std::cerr << "[DB] Error: " << err << "\n";
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