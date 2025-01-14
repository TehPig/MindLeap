//
// Created by TehPig on 1/5/2025.
//

#ifndef USERSTATS_HPP
#define USERSTATS_HPP

#include <QString>
#include <QDate>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Base/Stats.hpp"

class UserStats final : public Stats {
private:
    QString user_id;
    QDate date;
    int cards_seen;
    int pressed_again;
    int pressed_hard;
    int pressed_good;
    int pressed_easy;
    int time_spent_seconds;
    int times_used;

public:
    // Constructors
    UserStats(const QString& user_id, const QDate& date, const int& cards_seen, const int& pressed_again, const int& pressed_hard, const int& pressed_good, const int& pressed_easy, const int& time_spent_seconds, const int& times_used);
    UserStats(const QString& user_id);
    UserStats();

    // Getters
    int getCardsSeen() const;
    int getPressedAgain() const;
    int getPressedHard() const;
    int getPressedGood() const;
    int getPressedEasy() const;
    int getTimeSpentSeconds() const;
    int getTimesUsed() const;

    // Setters


    // Database Operations
    // Load stats from database
    Stats* loadStats() override;
    // Initialize stats to database
    bool initializeStats() const override;

    // Update stats based on user interactions
    void updateStats(const StatsUpdateContext& context) override;
    // Display stats for debugging
    void displayStats() const override;
};

#endif
