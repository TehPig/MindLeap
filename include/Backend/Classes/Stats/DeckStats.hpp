//
// Created by TehPig on 1/5/2025.
//

#ifndef DECKSTATS_HPP
#define DECKSTATS_HPP

#include <QString>
#include <QDate>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Base/Stats.hpp"

class DeckStats final : public Stats {
private:
    QString user_id;
    QString deck_id;
    QDate date;
    int cards_seen;
    int time_spent_seconds;

public:
    // Constructors
    DeckStats(const QString& user_id, const QString& deck_id, const QDate& date, const int& cards_seen, const int& time_spent_seconds);
    DeckStats(const QString& user_id, const QString& deck_id);
    DeckStats();

    // Getters
    int getCardsSeen() const;
    int getTimeSpentSeconds() const;

    // Setters
    void setDeckID(const QString& id) { deck_id = id; }
    void setUserID(const QString& id) { user_id = id; }

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
