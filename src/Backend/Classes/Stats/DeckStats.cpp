//
// Created by TehPig on 1/5/2025.
//

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
    const int& cards_seen,
    const qint64& time_spent_seconds,
    const qint64& session_start_time
    ) : user_id(user_id), deck_id(deck_id), date(date), cards_seen(cards_seen), time_spent_seconds(time_spent_seconds), session_start_time(session_start_time) {}
DeckStats::DeckStats(const QString& user_id, const QString& deck_id) : user_id(user_id), deck_id(deck_id), cards_seen(0), time_spent_seconds(0), session_start_time(0) {}
// Default constructor
DeckStats::DeckStats() : user_id(""), deck_id(""), date(QDate::currentDate()), cards_seen(0), time_spent_seconds(0), session_start_time(0) {}
// Getters
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

    qDebug() << "[DB - DeckStats] Loading stats for deck id:" << this->deck_id;

    query.prepare(QStringLiteral("SELECT * FROM DeckStats WHERE id = ?"));
    query.addBindValue(this->deck_id);

    if (!query.exec()) {
        qDebug() << "[DB - DeckStats] Failed to load stats:" << query.lastError().text();
        return {};
    }

    if (!query.next()) return {};

    this->user_id = query.value("user_id").toString();
    this->deck_id = query.value("id").toString();
    this->date = query.value("date").toDate();
    this->cards_seen = query.value("cards_seen").toInt();
    this->time_spent_seconds = query.value("time_spent_seconds").toInt();
    this->session_start_time = query.value("session_start_time").toInt();

    return new DeckStats(
        this->user_id,
        this->deck_id,
        this->date,
        this->cards_seen,
        this->time_spent_seconds,
        this->session_start_time
    );
}

// Initialize stats to database.
bool DeckStats::initialize() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Check if a record for the current date already exists
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM DeckStats WHERE id = ? AND date = DATE('now')"));
    query.addBindValue(this->deck_id);

    if (!query.exec()) {
        qDebug() << "[DB - DeckStats] Failed to check existing stats:" << query.lastError().text();
        return true;
    }
    if (query.next() && query.value(0).toInt() > 0) return true; // Stats already exist

        // Insert new stats entry for the current date
    query.prepare(QStringLiteral(
        "INSERT INTO DeckStats (id, user_id, date) "
        "VALUES (?, (SELECT id FROM SavedUser LIMIT 1), DATE('now'))"
    ));
    query.addBindValue(this->deck_id);

    if (!query.exec()) {
        qDebug() << "[DB - DeckStats] Failed to save stats for Deck:" << query.lastError().text();
        return false;
    }

    return true;
}

// Update stats based on user interactions
bool DeckStats::update(const StatsUpdateContext& context) {
    if (context.type != StatsUpdateType::Deck) {
        qDebug() << "[DB - DeckStats] Invalid context type.";
        return false;
    }

    QStringList updates;
    QList<QVariant> bindValues;

    // Collect updates and bind values
    if (context.deck.card_added) {
        updates << "cards_added = cards_added + 1";
    }
    if (context.deck.cards_seen > 0) {
        updates << "cards_seen = cards_seen + ?";
        bindValues << context.deck.cards_seen;
    }
    if (context.deck.time_spent > 0) {
        updates << "time_spent_seconds = time_spent_seconds + ?";
        bindValues << context.deck.time_spent;
    }
    if (context.deck.start_study > 0) {
        updates << "session_start_time = ?";
        bindValues << QDateTime::currentDateTime().toSecsSinceEpoch();
    }

    // No updates to perform
    if (updates.isEmpty()) {
        qDebug() << "[DB - DeckStats] No updates to perform.";
        return false;
    }

    // Construct query
    QString queryString = QString("UPDATE DeckStats SET %1 WHERE id = ? AND date = DATE('now')")
                          .arg(updates.join(", "));
    bindValues << this->deck_id; // Add the deck_id for the WHERE clause

    // Debugging query and bind values
    // qDebug() << "[DB - DeckStats] Query String:" << queryString;
    // qDebug() << "[DB - DeckStats] Bound Values:" << bindValues;

    // Prepare query
    QSqlQuery query(Database::getInstance()->getDB());
    query.prepare(queryString);

    // Bind values
    for (const auto& value : bindValues) {
        query.addBindValue(value);
    }

    // Execute query
    if (!query.exec()) {
        qDebug() << "[DB - DeckStats] Failed to update stats:" << query.lastError().text();
        return false;
    }

    return true;
}

// Display stats for debugging
void DeckStats::display() const {
    qDebug() << QStringLiteral("User ID: %1, Deck ID: %2, Cards Seen: %4, Time Spent: %5")
                .arg(user_id)
                .arg(deck_id)
                .arg(cards_seen)
                .arg(time_spent_seconds);
}
