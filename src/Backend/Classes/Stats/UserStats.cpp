//
// Created by TehPig on 1/5/2025.
//

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Stats/UserStats.hpp"

// Constructors
UserStats::UserStats(const QString& user_id) : user_id(user_id), cards_seen(0), pressed_again(0), pressed_hard(0), pressed_good(0), pressed_easy(0), time_spent_seconds(0), times_used(0) {}

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
// Update stats based on user interactions
void UserStats::updateStats(const StatsUpdateContext& context) {
    // cards_seen++;
    // time_spent_seconds += value;
    // times_used++;
}

// Display stats for debugging
void UserStats::displayStats() const {
    qDebug() << "User ID: " << user_id;
    qDebug() << "Cards Seen: " << cards_seen;
    qDebug() << "Pressed Again: " << pressed_again;
    qDebug() << "Pressed Hard: " << pressed_hard;
    qDebug() << "Pressed Good: " << pressed_good;
    qDebug() << "Pressed Easy: " << pressed_easy;
    qDebug() << "Time Spent: " << time_spent_seconds;
    qDebug() << "Times Used: " << times_used;
}
