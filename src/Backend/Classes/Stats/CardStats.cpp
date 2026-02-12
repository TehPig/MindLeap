//
// Created by TehPig on 1/5/2025.
//

#include "Backend/Utilities/Logger.hpp"
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
CardStats::CardStats() : card_id(""), user_id(""), date(QDate::currentDate()), times_seen(0), time_spent_seconds(0), last_seen(0), easeFactor(2.5f), interval(0), repetitions(0), card_start_time(0) {}

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

    Logger::db("Loading card stats", QString("CardID: %1").arg(this->card_id));

    query.prepare(QStringLiteral("SELECT * FROM CardStats WHERE id = ? ORDER BY date DESC LIMIT 1"));
    query.addBindValue(this->card_id);

    if (!query.exec()) {
        Logger::error("Failed to load card stats: " + query.lastError().text(), "CardStats");
        return {};
    }
    if (!query.next()) return {};

    // Load stats into object
    this->card_id = query.value("id").toString();
    this->user_id = query.value("user_id").toString();
    this->date = query.value("date").toDate();
    this->times_seen = query.value("times_seen").toInt();
    this->time_spent_seconds = query.value("time_spent_seconds").toInt();
    this->last_seen = query.value("last_seen").toLongLong();
    this->easeFactor = query.value("ease_factor").toFloat();
    this->interval = query.value("interval").toInt();
    this->repetitions = query.value("repetitions").toInt();
    this->card_start_time = query.value("card_start_time").toLongLong();

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

Stats* CardStats::loadTotal() {
    return load(); // For now, CardStats are row-per-day but we usually only care about latest.
}

// Initialize stats to database.
bool CardStats::initialize() const {
    if (this->card_id.isEmpty()) {
        Logger::error("Cannot initialize stats for an empty card ID", "CardStats");
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Check if a record for the current date already exists
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM CardStats WHERE id = ? AND date = DATE('now')"));
    query.addBindValue(this->card_id);

    if (!query.exec()) {
        Logger::error("Failed to check existing stats: " + query.lastError().text(), "CardStats");
        return false;
    }
    if (query.next() && query.value(0).toInt() > 0) return true; // Stats already exist

    // Fetch the most recent stats to carry over
    query.prepare(QStringLiteral("SELECT ease_factor, interval, repetitions FROM CardStats WHERE id = ? ORDER BY date DESC LIMIT 1"));
    query.addBindValue(this->card_id);

    float latestEaseFactor = 2.5f;
    int latestInterval = 0;
    int latestRepetitions = 0;

    if (query.exec() && query.next()) {
        latestEaseFactor = query.value(0).toFloat();
        latestInterval = query.value(1).toInt();
        latestRepetitions = query.value(2).toInt();
    }

    // Insert new stats entry for the current date
    query.prepare(QStringLiteral("INSERT INTO CardStats (id, user_id, date, ease_factor, interval, repetitions) VALUES (?, (SELECT id FROM SavedUser LIMIT 1), DATE('now'), ?, ?, ?)"));
    query.addBindValue(this->card_id);
    query.addBindValue(latestEaseFactor);
    query.addBindValue(latestInterval);
    query.addBindValue(latestRepetitions);

    if (!query.exec()) {
        Logger::error("Failed to save stats for Card: " + query.lastError().text(), "CardStats");
        return false;
    }

    return true;
}

// Update stats based on user interactions
bool CardStats::update(const StatsUpdateContext& context) {
    if (context.type != StatsUpdateType::Card) {
        Logger::warn("Invalid context type for CardStats update", "CardStats");
        return false;
    }

    QStringList updates;
    QList<QVariant> bindValues;

    // Collect updates based on context flags
    if (context.card.update_start_study) {
        updates << "card_start_time = ?";
        this->card_start_time = QDateTime::currentDateTime().toSecsSinceEpoch();
        bindValues << this->card_start_time;
    }
    if (context.card.update_times_seen) {
        updates << "times_seen = times_seen + 1";
    }
    if (context.card.update_last_seen) {
        updates << "last_seen = ?";
        this->last_seen = QDateTime::currentDateTime().toSecsSinceEpoch();
        bindValues << this->last_seen;
    }
    if (context.card.update_time_spent) {
        updates << "time_spent_seconds = time_spent_seconds + ?";
        bindValues << context.card.time_spent_increment;
    }
    if (context.card.update_ease_factor) {
        updates << "ease_factor = ?";
        bindValues << this->easeFactor;
    }
    if (context.card.update_interval) {
        updates << "interval = ?";
        bindValues << this->interval;
    }
    if (context.card.update_repetitions) {
        updates << "repetitions = ?";
        bindValues << this->repetitions;
    }

    // No updates to perform
    if (updates.isEmpty()) {
        return true;
    }

    // Construct query
    QString queryString = QString("UPDATE CardStats SET %1 WHERE id = ? AND user_id = ? AND date = DATE('now')")
                          .arg(updates.join(", "));
    bindValues << this->card_id << this->user_id; // Add the card_id and user_id for the WHERE clause

    // Prepare query
    QSqlQuery query(Database::getInstance()->getDB());
    query.prepare(queryString);

    // Bind values
    for (const auto& value : bindValues) {
        query.addBindValue(value);
    }

    // Execute query
    if (!query.exec()) {
        Logger::error("Failed to update stats: " + query.lastError().text(), "CardStats");
        return false;
    }

    return true;
}

// Display stats for debugging
void CardStats::display() const {
    QString msg = QString("Times Seen: %1, Interval: %2, Ease: %3, Reps: %4")
                  .arg(QString::number(times_seen), QString::number(interval), QString::number(easeFactor), QString::number(repetitions));
    Logger::info(msg, "CardStats");
}
