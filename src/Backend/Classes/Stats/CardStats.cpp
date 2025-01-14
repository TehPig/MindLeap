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
CardStats::CardStats(
    const QString& card_id,
    const QString& user_id,
    const QDate& date,
    const int& times_seen,
    const int& time_spent_seconds,
    const QDateTime& last_seen,
    const float& easeFactor,
    const int& interval,
    const int& repetitions) : card_id(card_id), user_id(user_id), date(date), times_seen(times_seen), time_spent_seconds(time_spent_seconds), last_seen(last_seen), easeFactor(easeFactor), interval(interval), repetitions(repetitions) {}
CardStats::CardStats(const QString& card_id, const QString& user_id) : card_id(card_id), user_id(user_id), times_seen(0), time_spent_seconds(0) {}
// Default constructor
CardStats::CardStats() = default;

// Getters
QString CardStats::getCardID() const { return card_id; }

QString CardStats::getUserID() const { return user_id; }

int CardStats::getTimesSeen() const { return times_seen; }

int CardStats::getTimeSpentSeconds() const { return time_spent_seconds; }

QDateTime CardStats::getLastSeen() const { return last_seen; }

float CardStats::getEaseFactor() const { return easeFactor; }

int CardStats::getInterval() const { return interval; }

int CardStats::getRepetitions() const { return repetitions; }

// Setters


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
    this->last_seen = query.value("last_seen").toDateTime();
    this->easeFactor = query.value("easeFactor").toFloat();
    this->interval = query.value("interval").toInt();
    this->repetitions = query.value("repetitions").toInt();

    return new CardStats(
        this->card_id,
        this->user_id,
        this->date,
        this->times_seen,
        this->time_spent_seconds,
        this->last_seen,
        this->easeFactor,
        this->interval,
        this->repetitions
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
    qDebug() << QStringLiteral("Card ID: %1, User ID: %2, Times Seen: %3, Time Spent: %4, Last Seen: %5, Ease Factor: %6, Interval: %7, Repetitions: %8")
            .arg(card_id)
            .arg(user_id)
            .arg(times_seen)
            .arg(time_spent_seconds)
            .arg(last_seen.toString())
            .arg(easeFactor)
            .arg(interval)
            .arg(repetitions);
}