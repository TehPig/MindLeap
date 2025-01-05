//
// Created by TehPig on 1/5/2025.
//

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Stats/DeckStats.hpp"

// Constructors
DeckStats::DeckStats(const QString& user_id, const QString& deck_id) : user_id(user_id), deck_id(deck_id), total_cards(0), cards_seen(0), time_spent_seconds(0), average_interval(0) {}

// Getters
int DeckStats::getTotalCards() const { return total_cards; }

int DeckStats::getCardsSeen() const { return cards_seen; }

int DeckStats::getTimeSpentSeconds() const { return time_spent_seconds; }

int DeckStats::getAverageInterval() const { return average_interval; }

// Setters


// Database Operations
// Update stats based on user interactions
void DeckStats::updateStats(const StatsUpdateContext& context) {
    // cards_seen++;
    // time_spent_seconds += value;
}

// Display stats for debugging
void DeckStats::displayStats() const {
    qDebug() << "User ID: " << user_id;
    qDebug() << "Deck ID: " << deck_id;
    qDebug() << "Total Cards: " << total_cards;
    qDebug() << "Cards Seen: " << cards_seen;
    qDebug() << "Time Spent: " << time_spent_seconds;
    qDebug() << "Average Interval: " << average_interval;
}
