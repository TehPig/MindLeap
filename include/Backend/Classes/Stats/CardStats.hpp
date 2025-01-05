//
// Created by TehPig on 1/5/2025.
//

#ifndef CARDSTATS_HPP
#define CARDSTATS_HPP

#include <QString>
#include <QDateTime>

#include "Backend/Classes/Base/Stats.hpp"

class CardStats final : public Stats {
private:
    QString card_id;
    QString user_id;
    int times_seen;
    int time_spent_seconds;
    int time_to_reappear;
    QDateTime last_seen;

public:
    // Constructors
    CardStats(const QString& card_id, const QString& user_id);

    // Getters
    QString getCardID() const;
    QString getUserID() const;
    int getTimesSeen() const;
    int getTimeSpentSeconds() const;
    int getTimeToReappear() const;
    QDateTime getLastSeen() const;

    // Setters


    // Database Operations
    // Update stats based on user interactions
    void updateStats(const StatsUpdateContext& context) override;
    // Display stats for debugging
    void displayStats() const override;
};

#endif
