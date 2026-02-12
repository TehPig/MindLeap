//
// Created by TehPig on 1/5/2025.
//

#include "Backend/Utilities/Logger.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Stats/DeckStats.hpp"
#include "Backend/Database/setup.hpp"

// Constructors
DeckStats::DeckStats(
    const QString& user_id,
    const QString& deck_id,
    const QDate& date,
    const int& cards_added,
    const int& cards_seen,
    const qint64& time_spent_seconds,
    const qint64& session_start_time
    ) : user_id(user_id), deck_id(deck_id), date(date), cards_added(cards_added), cards_seen(cards_seen), time_spent_seconds(time_spent_seconds), session_start_time(session_start_time) {}
DeckStats::DeckStats(const QString& user_id, const QString& deck_id) : user_id(user_id), deck_id(deck_id), date(QDate::currentDate()), cards_added(0), cards_seen(0), time_spent_seconds(0), session_start_time(0) {}
// Default constructor
DeckStats::DeckStats() : user_id(""), deck_id(""), date(QDate::currentDate()), cards_added(0), cards_seen(0), time_spent_seconds(0), session_start_time(0) {}
// Getters
int DeckStats::getCardsAdded() const { return cards_added; }

int DeckStats::getCardsSeen() const { return cards_seen; }

qint64 DeckStats::getSessionStartTime() const { return session_start_time ; }

qint64 DeckStats::getTimeSpent() const { return time_spent_seconds; }

// Setters
// Set Deck ID
void DeckStats::setDeckID(const QString& id) { this->deck_id = id; }

// Get User ID
void DeckStats::setUserID(const QString& id) { this->user_id = id; }

// Database Operations
// Load stats from database
// Using the reserved keyword "new", clearing memory is required on the frontend
Stats* DeckStats::load() {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch current user ID
    QSqlQuery userQuery(db->getDB());
    userQuery.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!userQuery.exec() || !userQuery.next()) return {};
    const QString currentUserID = userQuery.value(0).toString();

    Logger::db("Loading deck stats", QString("DeckID: %1").arg(this->deck_id));

    query.prepare(QStringLiteral("SELECT * FROM DeckStats WHERE id = ? AND user_id = ? ORDER BY date DESC LIMIT 1"));
    query.addBindValue(this->deck_id);
    query.addBindValue(currentUserID);

    if (!query.exec()) {
        Logger::error("Failed to load deck stats: " + query.lastError().text(), "DeckStats");
        return {};
    }

    if (!query.next()) return {};

    this->user_id = query.value("user_id").toString();
    this->deck_id = query.value("id").toString();
    this->date = query.value("date").toDate();
    this->cards_added = query.value("cards_added").toInt();
    this->cards_seen = query.value("cards_seen").toInt();
    this->time_spent_seconds = query.value("time_spent_seconds").toLongLong();
    this->session_start_time = query.value("session_start_time").toLongLong();

    return new DeckStats(
        this->user_id,
        this->deck_id,
        this->date,
        this->cards_added,
        this->cards_seen,
        this->time_spent_seconds,
        this->session_start_time
    );
}

Stats* DeckStats::loadTotal() {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    QSqlQuery userQuery(db->getDB());
    userQuery.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!userQuery.exec() || !userQuery.next()) return {};
    const QString currentUserID = userQuery.value(0).toString();

    query.prepare(QStringLiteral(R"(
        SELECT SUM(cards_added), SUM(cards_seen), SUM(time_spent_seconds)
        FROM DeckStats 
        WHERE id = ? AND user_id = ?
    )"));
    query.addBindValue(this->deck_id);
    query.addBindValue(currentUserID);

    if (!query.exec() || !query.next()) return {};

    this->user_id = currentUserID;
    this->cards_added = query.value(0).toInt();
    this->cards_seen = query.value(1).toInt();
    this->time_spent_seconds = query.value(2).toLongLong();
    this->date = QDate::currentDate(); // Use today as a placeholder

    return new DeckStats(
        this->user_id,
        this->deck_id,
        this->date,
        this->cards_added,
        this->cards_seen,
        this->time_spent_seconds,
        0
    );
}

// Initialize stats to database.
bool DeckStats::initialize() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch current user ID
    QSqlQuery userQuery(db->getDB());
    userQuery.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!userQuery.exec() || !userQuery.next()) return false;
    const QString currentUserID = userQuery.value(0).toString();

    // Check if a record for the current date already exists
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM DeckStats WHERE id = ? AND user_id = ? AND date = DATE('now')"));
    query.addBindValue(this->deck_id);
    query.addBindValue(currentUserID);

    if (!query.exec()) {
        Logger::error("Failed to check existing stats: " + query.lastError().text(), "DeckStats");
        return false;
    }
    if (query.next() && query.value(0).toInt() > 0) return true; // Stats already exist

        // Insert new stats entry for the current date
    query.prepare(QStringLiteral(
        "INSERT INTO DeckStats (id, user_id, date) "
        "VALUES (?, ?, DATE('now'))"
    ));
    query.addBindValue(this->deck_id);
    query.addBindValue(currentUserID);

    if (!query.exec()) {
        Logger::error("Failed to save stats for Deck: " + query.lastError().text(), "DeckStats");
        return false;
    }

    return true;
}

// Update stats based on user interactions
bool DeckStats::update(const StatsUpdateContext& context) {
    const Database* db = Database::getInstance();

    if (context.type != StatsUpdateType::Deck) {
        Logger::warn("Invalid context type for DeckStats update", "DeckStats");
        return false;
    }

    QStringList updates;
    QList<QVariant> bindValues;

    // Collect updates and bind values
    if (context.deck.update_card_added) {
        updates << "cards_added = cards_added + 1";
    }
    if (context.deck.update_cards_seen) {
        updates << "cards_seen = cards_seen + 1";
    }
    if (context.deck.update_time_spent) {
        updates << "time_spent_seconds = time_spent_seconds + ?";
        bindValues << context.deck.time_spent_increment;
    }
    if (context.deck.update_start_study) {
        updates << "session_start_time = ?";
        this->session_start_time = QDateTime::currentSecsSinceEpoch();
        bindValues << this->session_start_time;
    }

    // No updates to perform
    if (updates.isEmpty()) {
        return true;
    }

    // Fetch current user ID
    QSqlQuery userQuery(db->getDB());
    userQuery.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!userQuery.exec() || !userQuery.next()) return false;
    const QString currentUserID = userQuery.value(0).toString();

    // Construct query
    QString queryString = QString("UPDATE DeckStats SET %1 WHERE id = ? AND user_id = ? AND date = DATE('now')")
                          .arg(updates.join(", "));
    bindValues << this->deck_id << currentUserID; // Add the deck_id and user_id for the WHERE clause

    // Prepare query
    QSqlQuery query(Database::getInstance()->getDB());
    query.prepare(queryString);

    // Bind values
    for (const auto& value : bindValues) {
        query.addBindValue(value);
    }

    // Execute query
    if (!query.exec()) {
        Logger::error("Failed to update deck stats: " + query.lastError().text(), "DeckStats");
        return false;
    }

    return true;
}

// Display stats for debugging
void DeckStats::display() const {
    qDebug() << QStringLiteral("User ID: %1, Deck ID: %2, Cards Added: %3, Cards Seen: %4, Time Spent: %5")
                .arg(user_id, deck_id)
                .arg(cards_added)
                .arg(cards_seen)
                .arg(time_spent_seconds);
}
