//
// Created by TehPig on 11/27/2024.
//

#ifndef SETUP_H
#define SETUP_H

#include <string>
#include <sqlite3.h>

class Database {
    private:
        sqlite3 *db;
        std::string path;

    public:
        Database();
        Database(const std::string &path);
        ~Database();

        void iniitalize();
        bool execute(const std::string &query);
};

#endif