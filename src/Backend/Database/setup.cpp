#include <iostream>

#include <QSqlQuery>
#include <QSqlError>

#include "Backend/Database/setup.hpp"
#include "Backend/Database/queries.hpp"

// Define static members
std::unique_ptr<Database> Database::instance;
std::once_flag Database::initInstanceFlag;

Database::Database(const std::string &path) : db(QSqlDatabase::addDatabase("QSQLITE")), path(path) {
    std::cerr << "[Database] Constructor called with path: " << path << "\n";
    db.setDatabaseName(QString::fromStdString(path));
}

Database::~Database() {
    if (db.isOpen()) db.close();
}

Database* Database::getInstance(const std::string &path) {
    std::call_once(initInstanceFlag, [&]() {
        std::cerr << "[Database] Initializing instance with path: " << path << "\n";
        instance.reset(new Database(path));
    });
    return instance.get();
}

QSqlDatabase Database::getDB() const {
    std::cerr << "[Database] getDB called\n";
    return db;
}

void Database::initialize() {
    std::cerr << "[Database] Initializing database\n";
    if (!db.open()) {
        throw std::runtime_error("[Database] Could not open database: " + db.lastError().text().toStdString());
    }

    // Create tables
    if (!execute(CREATE_USERS_TABLE) ||
        !execute(CREATE_DECKS_TABLE) ||
        !execute(CREATE_CARDS_TABLE) ||
        !execute(CREATE_DECKS_CARDS_TABLE) ||
        !execute(CREATE_SAVED_USER_TABLE)) {
        throw std::runtime_error("[Database] Failed to initialize database.");
    }

    std::cerr << "[Database] Initialized successfully!\n";
}

bool Database::prepare(const std::string &query, const QVariantList &params) const {
    std::cerr << "[Database] Preparing query: " << query << "\n";
    QSqlQuery sqlQuery(db);
    if (!sqlQuery.prepare(QString::fromStdString(query))) {
        std::cerr << "[DB] Query preparation failed: " << query << "\n";
        std::cerr << "[DB] Error: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    // Bind parameters
    for (const auto &param : params) {
        sqlQuery.addBindValue(param);
    }

    // Execute the query
    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Query execution failed: " << query << "\n";
        std::cerr << "[DB] Error: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    return true;
}

bool Database::execute(const std::string &query) const {
    std::cerr << "[Database] Executing query: " << query << "\n";
    if (QSqlQuery sqlQuery(db); !sqlQuery.exec(QString::fromStdString(query))) {
        std::cerr << "[DB] Query execution failed: " << query << "\n";
        std::cerr << "[DB] Error: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    return true;
}