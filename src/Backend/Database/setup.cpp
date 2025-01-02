#include <iostream>

#include <QSqlQuery>
#include <QSqlError>

#include "Backend/Database/setup.hpp"
#include "Backend/Database/queries.hpp"

// Define static members
std::unique_ptr<Database> Database::instance;
std::once_flag Database::initInstanceFlag;

Database::Database(const std::string &path) : db(QSqlDatabase::addDatabase("QSQLITE")), path(path) {
    db.setDatabaseName(QString::fromStdString(path));
}

Database::~Database() {
    if (db.isOpen()) db.close();
}

Database* Database::getInstance(const std::string &path) {
    std::call_once(initInstanceFlag, [&]() {
        instance.reset(new Database(path));
    });
    return instance.get();
}

QSqlDatabase Database::getDB() const {
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
        !execute(CREATE_USERS_DECKS_TABLE) ||
        !execute(CREATE_SAVED_USER_TABLE) ||
        !execute(CREATE_USER_STATS_TABLE) ||
        !execute(CREATE_CARD_STATS_TABLE) ||
        !execute(CARD_STATS_CARD_INDEX) ||
        !execute(CARD_STATS_USER_INDEX) ||
        !execute(CARD_STATS_DATE_INDEX)) {
        throw std::runtime_error("[Database] Failed to initialize database.");
    }

    std::cerr << "[Database] Initialized successfully!\n";
}

bool Database::prepare(const std::string &query, const QVariantList &params) const {
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
    if (QSqlQuery sqlQuery(db); !sqlQuery.exec(QString::fromStdString(query))) {
        std::cerr << "[DB] Query execution failed: " << query << "\n";
        std::cerr << "[DB] Error: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    return true;
}