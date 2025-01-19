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
    const int& time_spent_seconds) : user_id(user_id), date(date), cards_seen(cards_seen), pressed_again(pressed_again), pressed_hard(pressed_hard), pressed_good(pressed_good), pressed_easy(pressed_easy), time_spent_seconds(time_spent_seconds) {}
UserStats::UserStats(const QString& user_id) : user_id(user_id), cards_seen(0), pressed_again(0), pressed_hard(0), pressed_good(0), pressed_easy(0), time_spent_seconds(0) {}
// Default constructor
UserStats::UserStats()
    : user_id(""),
      date(QDate::currentDate()),
      cards_seen(0),
      pressed_again(0),
      pressed_hard(0),
      pressed_good(0),
      pressed_easy(0),
      time_spent_seconds(0) {}

// Getters
int UserStats::getCardsSeen() const { return cards_seen; }

int UserStats::getPressedAgain() const { return pressed_again; }

int UserStats::getPressedHard() const { return pressed_hard; }

int UserStats::getPressedGood() const { return pressed_good; }

int UserStats::getPressedEasy() const { return pressed_easy; }

int UserStats::getTimeSpentSeconds() const { return time_spent_seconds; }

// Setters
void UserStats::setUserID(const QString& user_id) { this->user_id = user_id; }

// Database Operations
// Load stats from database
// Using the reserved keyword "new", clearing memory is required on the frontend
Stats* UserStats::load() {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM UserStats WHERE id = ?"));
    query.addBindValue(this->user_id);

    if (!query.exec()) {
        qDebug() << "[DB - UserStats] Failed to load stats:" << query.lastError().text();
        return {};
    }
    if (!query.next()) return {};

    this->user_id = query.value("id").toString();
    this->date = query.value("date").toDate();
    this->cards_seen = query.value("cards_seen").toInt();
    this->pressed_again = query.value("pressed_again").toInt();
    this->pressed_hard = query.value("pressed_hard").toInt();
    this->pressed_good = query.value("pressed_good").toInt();
    this->pressed_easy = query.value("pressed_easy").toInt();
    this->time_spent_seconds = query.value("time_spent_seconds").toInt();

    return new UserStats(
        this->user_id,
        this->date,
        this->cards_seen,
        this->pressed_again,
        this->pressed_hard,
        this->pressed_good,
        this->pressed_easy,
        this->time_spent_seconds
    );
}

// Initialize stats to database.
bool UserStats::initialize() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Check if a record for the current date already exists
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM UserStats WHERE id = ? AND date = DATE('now')"));
    query.addBindValue(this->user_id);

    if (!query.exec()) {
        qDebug() << "[DB - UserStats] Failed to check existing stats:" << query.lastError().text();
        return true;
    }
    if (query.next() && query.value(0).toInt() > 0) return true; // Stats already exist

        // Insert new stats entry for the current date
        query.prepare(QStringLiteral("INSERT INTO UserStats (id, date) VALUES (?, DATE('now'))"));
        query.addBindValue(this->user_id);

        if (!query.exec()) {
            qDebug() << "[DB - UserStats] Failed to save stats for User:" << query.lastError().text();
            return false;
        }

    return true;
}

// Update stats based on user interactions
bool UserStats::update(const StatsUpdateContext& context) {
    if (context.type != StatsUpdateType::User) {
        qDebug() << "[DB - UserStats] Invalid context type.";
        return false;
    }

    QStringList updates;
    QList<QVariant> bindValues;

    // Collect updates and bind values for User
    if (context.user.pressed_again > 0) {
        updates << "pressed_again = pressed_again + ?";
        bindValues << context.user.pressed_again;
    }
    if (context.user.pressed_hard > 0) {
        updates << "pressed_hard = pressed_hard + ?";
        bindValues << context.user.pressed_hard;
    }
    if (context.user.pressed_good > 0) {
        updates << "pressed_good = pressed_good + ?";
        bindValues << context.user.pressed_good;
    }
    if (context.user.pressed_easy > 0) {
        updates << "pressed_easy = pressed_easy + ?";
        bindValues << context.user.pressed_easy;
    }
    if (context.user.time_spent > 0) {
        updates << "time_spent_seconds = time_spent_seconds + ?";
        bindValues << context.user.time_spent;
    }
    if (context.user.cards_seen > 0) {
        updates << "cards_seen = cards_seen + ?";
        bindValues << context.user.cards_seen;
    }

    // No updates to perform
    if (updates.isEmpty()) {
        qDebug() << "[DB - UserStats] No updates to perform.";
        return false;
    }

    // Construct query
    QString queryString = QString("UPDATE UserStats SET %1 WHERE id = ? AND date = DATE('now')")
                          .arg(updates.join(", "));
    bindValues << this->user_id; // Add the user_id for the WHERE clause

    // // Debugging query and bind values
    // qDebug() << "[DB - UserStats] Query String:" << queryString;
    // qDebug() << "[DB - UserStats] Bound Values:" << bindValues;

    // Prepare query
    QSqlQuery query(Database::getInstance()->getDB());
    query.prepare(queryString);

    // Bind values
    for (const auto& value : bindValues) {
        query.addBindValue(value);
    }

    // Execute query
    if (!query.exec()) {
        qDebug() << "[DB - UserStats] Failed to update stats:" << query.lastError().text();
        return false;
    }

    return true;
}

// Display stats for debugging
void UserStats::display() const {
    qDebug() << QStringLiteral("User ID: %1, Cards Seen: %2, Pressed Again: %3, Pressed Hard: %4, Pressed Good: %5, Pressed Easy: %6, Time Spent: %7, Times Used: %8")
            .arg(user_id)
            .arg(cards_seen)
            .arg(pressed_again)
            .arg(pressed_hard)
            .arg(pressed_good)
            .arg(pressed_easy)
            .arg(time_spent_seconds);
}
