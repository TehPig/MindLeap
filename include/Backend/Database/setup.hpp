#ifndef SETUP_HPP
#define SETUP_HPP

#include <string>
#include <memory>
#include <mutex>
#include <QSqlDatabase>
#include <QVariantList>

class Database {
private:
    static std::unique_ptr<Database> instance;
    static std::once_flag initInstanceFlag;
    QSqlDatabase db;
    std::string path;

public:
    Database(const std::string &path);
    ~Database();

    static Database* getInstance(const std::string &path = "app_data.db");
    QSqlDatabase getDB() const;
    void initialize();
};

#endif