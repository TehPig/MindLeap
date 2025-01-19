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
    const qint64& last_seen,
    const float& easeFactor,
    const int& interval,
    const int& repetitions,
    const qint64& card_start_time
    ) : card_id(card_id), user_id(user_id), date(date), times_seen(times_seen), time_spent_seconds(time_spent_seconds), last_seen(last_seen), easeFactor(easeFactor), interval(interval), repetitions(repetitions), card_start_time(card_start_time) {}
CardStats::CardStats(const QString& card_id, const QString& user_id) : card_id(card_id), user_id(user_id), times_seen(0), time_spent_seconds(0), card_start_time(0) {}
// Default constructor
CardStats::CardStats() : card_id(""), user_id(""), date(QDate::currentDate()), times_seen(0), time_spent_seconds(0), easeFactor(0.0f), interval(0), repetitions(0), card_start_time(0) {}
// Getters
QString CardStats::getCardID() const { return card_id; }

QString CardStats::getUserID() const { return user_id; }

int CardStats::getTimesSeen() const { return times_seen; }

int CardStats::getTimeSpent() const { return time_spent_seconds; }

qint64 CardStats::getCardStartTime() const { return card_start_time; }

qint64 CardStats::getLastSeen() const { return last_seen; }

float CardStats::getEaseFactor() const { return easeFactor; }

int CardStats::getInterval() const { return interval; }

int CardStats::getRepetitions() const { return repetitions; }

// Setters
// Set Card ID
void CardStats::setCardID(const QString &card_id){ this->card_id = card_id; };
// Set User ID
void CardStats::setUserID(const QString &user_id){ this->user_id = user_id; };

// Set Ease Factor (for SM-2 Algorithm)
void CardStats::setEaseFactor(const float easeFactor) { this->easeFactor = easeFactor; }

// Set Next Interval
void CardStats::setInterval(const int interval) { this->interval = interval; }

// Set Repetitions
void CardStats::setRepetitions(const int repetitions) { this->repetitions = repetitions; }

// Database Operations
// Load stats from database
// Using the reserved keyword "new", clearing memory is required on the frontend
Stats* CardStats::load() {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM CardStats WHERE id = ?"));
    query.addBindValue(this->card_id);

    if (!query.exec()) {
        qDebug() << "[DB - CardStats] Failed to load stats:" << query.lastError().text();
        return {};
    }
    if (!query.next()) return {};

    // Load stats into object - ADD PENDING STATS
    this->card_id = query.value("id").toString();
    this->user_id = query.value("user_id").toString();
    this->date = query.value("date").toDate();
    this->times_seen = query.value("times_seen").toInt();
    this->time_spent_seconds = query.value("time_spent_seconds").toInt();
    this->last_seen = query.value("last_seen").toInt();
    this->easeFactor = query.value("ease_factor").toFloat();
    this->interval = query.value("interval").toInt();
    this->repetitions = query.value("repetitions").toInt();
    this->repetitions = query.value("card_start_time").toInt();

    return new CardStats(
        this->card_id,
        this->user_id,
        this->date,
        this->times_seen,
        this->time_spent_seconds,
        this->last_seen,
        this->easeFactor,
        this->interval,
        this->repetitions,
        this->card_start_time
    );
}

// Initialize stats to database.
bool CardStats::initialize() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Check if a record for the current date already exists
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM CardStats WHERE id = ? AND date = DATE('now')"));
    query.addBindValue(this->card_id);

    if (!query.exec()) {
        qDebug() << "[DB - CardStats] Failed to check existing stats:" << query.lastError().text();
        return true;
    }
    if (query.next() && query.value(0).toInt() > 0) return true; // Stats alrea

        // Insert new stats entry for the current date
        query.prepare(QStringLiteral("INSERT INTO CardStats (id, user_id, date) VALUES (?, (SELECT id FROM SavedUser), DATE('now'))"));
        query.addBindValue(this->card_id);

        if (!query.exec()) {
            qDebug() << "[DB - CardStats] Failed to save stats for Card:" << query.lastError().text();
            return false;
        }

    return true;
}

// Update stats based on user interactions
bool CardStats::update(const StatsUpdateContext& context) {
    if (context.type != StatsUpdateType::Card) {
        qDebug() << "[DB - CardStats] Invalid context type.";
        return false;
    }

    QStringList updates;
    QList<QVariant> bindValues;

    // Collect updates and bind values for Card
    if (context.card.start_study) {
        updates << "card_start_time = ?";
        bindValues << QDateTime::currentDateTime().toSecsSinceEpoch();
    }
    if (context.card.times_seen) {
        updates << "times_seen = times_seen + 1";
    }
    if (context.card.last_seen) {
        updates << "last_seen = ?";
        bindValues << QDateTime::currentDateTime().toSecsSinceEpoch();
    }
    if (context.card.time_spent > 0) {
        updates << "time_spent_seconds = time_spent_seconds + ?";
        bindValues << context.card.time_spent;
    }
    if (context.card.ease_factor > 0) {
        updates << "ease_factor = ?";
        bindValues << context.card.ease_factor;
    }
    if (context.card.interval > 0) {
        updates << "interval = ?";
        bindValues << context.card.interval;
    }
    if (context.card.repetitions > 0) {
        updates << "repetitions = ?";
        bindValues << context.card.repetitions;
    }

    // No updates to perform
    if (updates.isEmpty()) {
        qDebug() << "[DB - CardStats] No updates to perform.";
        return false;
    }

    // Construct query
    QString queryString = QString("UPDATE CardStats SET %1 WHERE id = ? AND user_id = ? AND date = DATE('now')")
                          .arg(updates.join(", "));
    bindValues << this->card_id << this->user_id; // Add the card_id and user_id for the WHERE clause

    // Debugging query and bind values
    // qDebug() << "[DB - CardStats] Query String:" << queryString;
    // qDebug() << "[DB - CardStats] Bound Values:" << bindValues;

    // Prepare query
    QSqlQuery query(Database::getInstance()->getDB());
    query.prepare(queryString);

    // Bind values
    for (const auto& value : bindValues) {
        query.addBindValue(value);
    }

    // Execute query
    if (!query.exec()) {
        qDebug() << "[DB - CardStats] Failed to update stats:" << query.lastError().text();
        return false;
    }

    return true;
}

// Display stats for debugging
void CardStats::display() const {
    qDebug() << QStringLiteral("Card ID: %1, User ID: %2, Times Seen: %3, Time Spent: %4, Last Seen: %5, Ease Factor: %6, Interval: %7, Repetitions: %8")
            .arg(card_id)
            .arg(user_id)
            .arg(times_seen)
            .arg(time_spent_seconds)
            .arg(last_seen)
            .arg(easeFactor)
            .arg(interval)
            .arg(repetitions);
}
