//
// Created by TehPig on 1/5/2025.
//

#ifndef DECKSTATS_HPP
#define DECKSTATS_HPP

#include <QString>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Base/Stats.hpp"

class DeckStats final : public Stats {
private:
    QString user_id;
    QString deck_id;
    int total_cards;
    int cards_seen;
    int time_spent_seconds;
    int average_interval;

public:
    // Constructors
    DeckStats(const QString& user_id, const QString& deck_id);

    // Getters
    int getTotalCards() const;
    int getCardsSeen() const;
    int getTimeSpentSeconds() const;
    int getAverageInterval() const;

    // Setters


    // Database Operations
    // Update stats based on user interactions
    void updateStats(const StatsUpdateContext& context) override;
    // Display stats for debugging
    void displayStats() const override;
};

#endif
