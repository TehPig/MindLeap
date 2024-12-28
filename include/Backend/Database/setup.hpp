//
// Created by TehPig on 11/27/2024.
//

#ifndef SETUP_H
#define SETUP_H

#include <QSqlDatabase>
#include <QString>

#include <string>

class Database {
    private:
        static Database *instance;
        QSqlDatabase db;
        std::string path;

    public:
        Database(const std::string &path);
        ~Database();

        // Getter
        QSqlDatabase getDB() const;

        void initialize();
        bool prepare(const QString &query, const QVariantList &stringParams) const;
        bool execute(const QString &query) const;

        static Database* getInstance(const std::string &path = "app_data.db");
};

#endif
