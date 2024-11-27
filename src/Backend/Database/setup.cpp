//
// Created by TehPig on 11/27/2024.
//
#include <sqlite3.h>
#include <Backend/Database/setup.hpp>

#include <iostream>
#include <string>

Database::Database() : db(nullptr) {}
Database::Database(const std::string &path) : path(path) {}
Database::~Database() {
    if (db) sqlite3_close(db);
}

void Database::iniitalize() {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << "\n";
        std::cout << "[Database] Could not open database.";
    }

    std::cout << "[Database] Initialized successfully!";
}

bool Database::execute(const std::string &query) {
    char *err;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "Error: " << err << "\n";
        sqlite3_free(err);
        return false;
    }

    return true;
}