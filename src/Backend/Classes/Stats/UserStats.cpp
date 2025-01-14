//
// Created by TehPig on 1/5/2025.
//

#include <QSqlError>
#include <QSqlQuery>
#include <QDate>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Stats/UserStats.hpp"
#include "Backend/Database/setup.hpp"

// Constructors
UserStats::UserStats(
    const QString& user_id,
    const QDate& date,
    const int& cards_seen,
    const int& pressed_again,
    const int& pressed_hard,
    const int& pressed_good,
    const int& pressed_easy,
    const int& time_spent_seconds,
    const int& times_used) : user_id(user_id), date(date), cards_seen(cards_seen), pressed_again(pressed_again), pressed_hard(pressed_hard), pressed_good(pressed_good), pressed_easy(pressed_easy), time_spent_seconds(time_spent_seconds), times_used(times_used) {}
UserStats::UserStats(const QString& user_id) : user_id(user_id), cards_seen(0), pressed_again(0), pressed_hard(0), pressed_good(0), pressed_easy(0), time_spent_seconds(0), times_used(0) {}
// Default constructor
UserStats::UserStats() = default;

// Getters
int UserStats::getCardsSeen() const { return cards_seen; }

int UserStats::getPressedAgain() const { return pressed_again; }

int UserStats::getPressedHard() const { return pressed_hard; }

int UserStats::getPressedGood() const { return pressed_good; }

int UserStats::getPressedEasy() const { return pressed_easy; }

int UserStats::getTimeSpentSeconds() const { return time_spent_seconds; }

int UserStats::getTimesUsed() const { return times_used; }

// Setters


// Database Operations
// Load stats from database
// Using the reserved keyword "new", clearing memory is required on the frontend
Stats* UserStats::loadStats() {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM UserStats WHERE id = ?"));
    query.addBindValue(this->user_id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB - UserStats] Failed to load stats:" << query.lastError().text();
        return {};
    }

    this->user_id = query.value("id").toString();
    this->date = query.value("date").toDate();
    this->cards_seen = query.value("cards_seen").toInt();
    this->pressed_again = query.value("pressed_again").toInt();
    this->pressed_hard = query.value("pressed_hard").toInt();
    this->pressed_good = query.value("pressed_good").toInt();
    this->pressed_easy = query.value("pressed_easy").toInt();
    this->time_spent_seconds = query.value("time_spent_seconds").toInt();
    this->times_used = query.value("times_used").toInt();

    return new UserStats(
        this->user_id,
        this->date,
        this->cards_seen,
        this->pressed_again,
        this->pressed_hard,
        this->pressed_good,
        this->pressed_easy,
        this->time_spent_seconds,
        this->times_used
    );
}

// Initialize stats to database.
bool UserStats::initializeStats() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Check if a record for the current date already exists
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM UserStats WHERE id = ? AND date = DATE('now')"));
    query.addBindValue(this->user_id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB - UserStats] Failed to check existing stats:" << query.lastError().text();
        return false;
    }

    int count = query.value(0).toInt();
    if (count == 0) {
        // Insert new stats entry for the current date
        query.prepare(QStringLiteral("INSERT INTO UserStats (id, date) VALUES (?, DATE('now'))"));
        query.addBindValue(this->user_id);

        if (!query.exec()) {
            qDebug() << "[DB - UserStats] Failed to save stats for User: " << query.lastError().text();
            return false;
        }
    }

    return true;
}

// Update stats based on user interactions
void UserStats::updateStats(const StatsUpdateContext& context) {
    cards_seen++;
    time_spent_seconds += context.time_spent;
    times_used++;
}

// Display stats for debugging
void UserStats::displayStats() const {
    qDebug() << QStringLiteral("User ID: %1, Cards Seen: %2, Pressed Again: %3, Pressed Hard: %4, Pressed Good: %5, Pressed Easy: %6, Time Spent: %7, Times Used: %8")
            .arg(user_id)
            .arg(cards_seen)
            .arg(pressed_again)
            .arg(pressed_hard)
            .arg(pressed_good)
            .arg(pressed_easy)
            .arg(time_spent_seconds)
            .arg(times_used);
}
