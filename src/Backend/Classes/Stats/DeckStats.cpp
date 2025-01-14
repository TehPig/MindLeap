//
// Created by TehPig on 1/5/2025.
//

#include <QSqlQuery>
#include <QSqlError>
#include <QDate>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Stats/DeckStats.hpp"
#include "Backend/Classes/Deck.hpp"
#include "Backend/Database/setup.hpp"

// Constructors
DeckStats::DeckStats(
    const QString& user_id,
    const QString& deck_id,
    const QDate& date,
    const int& cards_seen,
    const int& time_spent_seconds) : user_id(user_id), deck_id(deck_id), date(date), cards_seen(cards_seen), time_spent_seconds(time_spent_seconds) {}
DeckStats::DeckStats(const QString& user_id, const QString& deck_id) : user_id(user_id), deck_id(deck_id), cards_seen(0), time_spent_seconds(0) {}
// Default constructor
DeckStats::DeckStats() = default;

// Getters
int DeckStats::getCardsSeen() const { return cards_seen; }

int DeckStats::getTimeSpentSeconds() const { return time_spent_seconds; }

// Setters


// Database Operations
// Load stats from database
// Using the reserved keyword "new", clearing memory is required on the frontend
Stats* DeckStats::loadStats() {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM DeckStats WHERE id = ?"));
    query.addBindValue(this->deck_id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB - DeckStats] Failed to load stats:" << query.lastError().text();
        return {};
    }

    this->user_id = query.value("user_id").toString();
    this->deck_id = query.value("id").toString();
    this->date = query.value("date").toDate();
    this->cards_seen = query.value("cards_seen").toInt();
    this->time_spent_seconds = query.value("time_spent_seconds").toInt();

    return new DeckStats(
        this->user_id,
        this->deck_id,
        this->date,
        this->cards_seen,
        this->time_spent_seconds
    );
}

// Initialize stats to database.
bool DeckStats::initializeStats() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Check if a record for the current date already exists
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM DeckStats WHERE id = ? AND date = DATE('now')"));
    query.addBindValue(this->deck_id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB - DeckStats] Failed to check existing stats:" << query.lastError().text();
        return false;
    }

    int count = query.value(0).toInt();
    if (count == 0) {
        // Insert new stats entry for the current date
        query.prepare(QStringLiteral("INSERT INTO DeckStats (id, date) VALUES (?, DATE('now'))"));
        query.addBindValue(this->deck_id);

        if (!query.exec()) {
            qDebug() << "[DB - DeckStats] Failed to save stats for Deck: " << query.lastError().text();
            return false;
        }
    }

    return true;
}

// Update stats based on user interactions
void DeckStats::updateStats(const StatsUpdateContext& context) {
    cards_seen++;
    time_spent_seconds += context.time_spent;
}

// Display stats for debugging
void DeckStats::displayStats() const {
    qDebug() << QStringLiteral("User ID: %1, Deck ID: %2, Cards Seen: %4, Time Spent: %5")
                .arg(user_id)
                .arg(deck_id)
                .arg(cards_seen)
                .arg(time_spent_seconds);
}
