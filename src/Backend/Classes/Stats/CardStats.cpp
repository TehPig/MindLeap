//
// Created by TehPig on 1/5/2025.
//

#include <QDebug>
#include <QDate>
#include <QSqlError>
#include <QSqlQuery>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Stats/CardStats.hpp"
#include "Backend/Database/setup.hpp"

// Constructors
CardStats::CardStats(const QString& card_id, const QString& user_id, const QDate& date, const int& times_seen, const int& time_spent_seconds, const int& time_to_reappear, const QDateTime& last_seen)
    : card_id(card_id), user_id(user_id), date(date), times_seen(times_seen), time_spent_seconds(time_spent_seconds), time_to_reappear(time_to_reappear), last_seen(last_seen) {}
CardStats::CardStats(const QString& card_id, const QString& user_id) : card_id(card_id), user_id(user_id), times_seen(0), time_spent_seconds(0), time_to_reappear(0) {}
// Default constructor
CardStats::CardStats() = default;

// Getters
QString CardStats::getCardID() const { return card_id; }

QString CardStats::getUserID() const { return user_id; }

int CardStats::getTimesSeen() const { return times_seen; }

int CardStats::getTimeSpentSeconds() const { return time_spent_seconds; }

int CardStats::getTimeToReappear() const { return time_to_reappear; }

QDateTime CardStats::getLastSeen() const { return last_seen; }

// Setters
int CardStats::setTimeToReappear(const int time) { return time_to_reappear = time; }

// Database Operations
// Load stats from database
// Using the reserved keyword "new", clearing memory is required on the frontend
Stats* CardStats::loadStats() {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM CardStats WHERE id = ?"));
    query.addBindValue(this->card_id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB - CardStats] Failed to load stats:" << query.lastError().text();
        return {};
    }

    // Load stats into object - ADD PENDING STATS
    this->card_id = query.value("id").toString();
    this->user_id = query.value("user_id").toString();
    this->date = query.value("date").toDate();
    this->times_seen = query.value("times_seen").toInt();
    this->time_spent_seconds = query.value("time_spent_seconds").toInt();
    this->time_to_reappear = query.value("time_to_reappear").toInt();
    this->last_seen = query.value("last_seen").toDateTime();

    return new CardStats(
        this->card_id,
        this->user_id,
        this->date,
        this->times_seen,
        this->time_spent_seconds,
        this->time_to_reappear,
        this->last_seen
    );
}

// Initialize stats to database.
bool CardStats::initializeStats() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Check if a record for the current date already exists
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM CardStats WHERE id = ? AND date = DATE('now')"));
    query.addBindValue(this->card_id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB - CardStats] Failed to check existing stats:" << query.lastError().text();
        return false;
    }

    int count = query.value(0).toInt();
    if (count == 0) {
        // Insert new stats entry for the current date
        query.prepare(QStringLiteral("INSERT INTO CardStats (id, date) VALUES (?, DATE('now'))"));
        query.addBindValue(this->card_id);

        if (!query.exec()) {
            qDebug() << "[DB - CardStats] Failed to save stats for Card: " << query.lastError().text();
            return false;
        }
    }

    return true;
}

// Update stats based on user interactions
void CardStats::updateStats(const StatsUpdateContext& context) {
    times_seen++;
    time_spent_seconds += context.time_spent;
    last_seen = QDateTime::currentDateTime();
}

// Display stats for debugging
void CardStats::displayStats() const {
    qDebug() << QStringLiteral("Card ID: %1, User ID: %2, Times Seen: %3, Time Spent: %4, Time to Reappear: %5, Last Seen: %6")
            .arg(card_id)
            .arg(user_id)
            .arg(times_seen)
            .arg(time_spent_seconds)
            .arg(time_to_reappear)
            .arg(last_seen.toString());
}