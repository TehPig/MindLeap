//
// Created by TehPig on 1/5/2025.
//

#ifndef CARDSTATS_HPP
#define CARDSTATS_HPP

#include <QString>
#include <QDateTime>
#include <QDate>

#include "Backend/Classes/Base/Stats.hpp"

class CardStats final : public Stats {
private:
    QString card_id;
    QString user_id;
    QDate date;
    int times_seen;
    int time_spent_seconds;
    int time_to_reappear;
    QDateTime last_seen;

public:
    // Constructors
    CardStats(const QString& card_id, const QString& user_id, const QDate& date, const int& times_seen, const int& time_spent_seconds, const int& time_to_reappear, const QDateTime& last_seen);
    CardStats(const QString& card_id, const QString& user_id);
    CardStats();

    // Getters
    QString getCardID() const;
    QString getUserID() const;
    int getTimesSeen() const;
    int getTimeSpentSeconds() const;
    int getTimeToReappear() const;
    QDateTime getLastSeen() const;

    // Setters
    int setTimeToReappear(int time);

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
