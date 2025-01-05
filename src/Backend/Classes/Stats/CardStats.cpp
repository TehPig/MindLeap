//
// Created by TehPig on 1/5/2025.
//

#include <QDebug>
#include <QDateTime>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Stats/CardStats.hpp"

// Constructors
CardStats::CardStats(const QString& card_id, const QString& user_id) : card_id(card_id), user_id(user_id), times_seen(0), time_spent_seconds(0), time_to_reappear(0) {}

// Getters
QString CardStats::getCardID() const { return card_id; }

QString CardStats::getUserID() const { return user_id; }

int CardStats::getTimesSeen() const { return times_seen; }

int CardStats::getTimeSpentSeconds() const { return time_spent_seconds; }

int CardStats::getTimeToReappear() const { return time_to_reappear; }

QDateTime CardStats::getLastSeen() const { return last_seen; }

// Setters


// Database Operations
// Update stats based on user interactions
void CardStats::updateStats(const StatsUpdateContext& context) {
    // times_seen++;
    // time_spent_seconds += time_spent;
    // last_seen = QDateTime::currentDateTime();

    // // Update time_to_reappear based on button_pressed
    // switch (button_pressed) {
    //     case 1:
    //         time_to_reappear = 0;
    //         break;
    //     case 2:
    //         time_to_reappear = 1;
    //         break;
    //     case 3:
    //         time_to_reappear = 3;
    //         break;
    //     case 4:
    //         time_to_reappear = 7;
    //         break;
    //     default:
    //         time_to_reappear = 0;
    //         break;
    // }
}

// Display stats for debugging
void CardStats::displayStats() const {
    qDebug() << "Card ID: " << card_id;
    qDebug() << "User ID: " << user_id;
    qDebug() << "Times Seen: " << times_seen;
    qDebug() << "Time Spent: " << time_spent_seconds;
    qDebug() << "Time to Reappear: " << time_to_reappear;
    qDebug() << "Last Seen: " << last_seen.toString();
}
