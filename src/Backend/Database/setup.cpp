#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

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
    qDebug() << "[DB] Initializing database";
    if (!db.open()) {
        qCritical() << "[DB] Could not open database:" + db.lastError().text();
        std::exit(EXIT_FAILURE);
    }

    // Create tables
    const std::vector<std::string> queries = {
        CREATE_USERS_TABLE,
        CREATE_DECKS_TABLE,
        CREATE_CARDS_TABLE,
        CREATE_DECKS_CARDS_TABLE,
        CREATE_USERS_DECKS_TABLE,
        CREATE_DECK_SETTINGS_TABLE,
        CREATE_SAVED_USER_TABLE,
        CREATE_USER_STATS_TABLE,
        CREATE_DECK_STATS_TABLE,
        CREATE_CARD_STATS_TABLE,
        CARD_STATS_CARD_INDEX,
        CARD_STATS_USER_INDEX,
        CARD_STATS_DATE_INDEX
    };

    for (const auto& query : queries) {
        QSqlQuery q;
        if (!q.exec(QString::fromStdString(query))) {
            qCritical() << "[DB] Failed to initialize database:" + q.lastError().text();
            std::exit(EXIT_FAILURE);
        }
    }

    qDebug() << "[DB] Initialized successfully!";
}

void Database::reset() {
    qDebug() << "[DB] Resetting database...";
    const std::vector<std::string> tables = {
        "CardStats",
        "DeckStats",
        "UserStats",
        "DecksCards",
        "UsersDecks",
        "DeckSettings",
        "SavedUser",
        "Cards",
        "Decks",
        "Users"
    };

    const std::vector<std::string> indexes = {
        "CARD_STATS_CARD_INDEX",
        "CARD_STATS_USER_INDEX",
        "CARD_STATS_DATE_INDEX"
    };

    for (const auto& index : indexes) {
        QSqlQuery q;
        if (!q.exec(QString("DROP INDEX IF EXISTS %1").arg(QString::fromStdString(index)))) {
            qCritical() << "[DB] Failed to drop index:" << q.lastError().text();
        }
    }

    // Drop existing tables
    for (const auto& table : tables) {
        QSqlQuery q;
        if (!q.exec(QString("DROP TABLE IF EXISTS %1").arg(QString::fromStdString(table)))) {
            qCritical() << "[DB] Failed to drop table" << QString::fromStdString(table) << ":" << q.lastError().text();
        } else {
            qDebug() << "[DB] Successfully dropped table" << QString::fromStdString(table);
        }
    }

    // Reinitialize the database
    initialize();
}