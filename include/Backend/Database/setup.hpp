//
// Created by TehPig on 11/27/2024.
//

#ifndef SETUP_H
#define SETUP_H

#include <sqlite3.h>

#include <string>

class Database {
    private:
        static Database *instance;
        sqlite3 *db;
        std::string path;

        Database();

    public:
        Database(const std::string &path);
        ~Database();

        // Getter
        sqlite3 *getDB() const;

        void initialize();
        bool execute(const std::string &query) const;

        static Database* getInstance(const std::string &path = "app_data.db");
};

#endif