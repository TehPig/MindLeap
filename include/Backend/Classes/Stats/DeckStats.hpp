//
// Created by TehPig on 1/5/2025.
//

#ifndef DECKSTATS_HPP
#define DECKSTATS_HPP

#include <QtGlobal>
#include <QString>
#include <QDate>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Base/Stats.hpp"

class DeckStats final : public Stats {
private:
    QString user_id;
    QString deck_id;
    QDate date;
    int cards_added;
    int cards_seen;
    qint64 time_spent_seconds;
    qint64 session_start_time;

public:
    // Constructors
    DeckStats(const QString& user_id, const QString& deck_id, const QDate& date, const int& cards_added, const int& cards_seen, const qint64& time_spent_seconds, const qint64& session_start_time);
    DeckStats(const QString& user_id, const QString& deck_id);
    DeckStats();

    // Getters
    QString getDeckID() const { return deck_id; }

    int getCardsAdded() const;
    int getCardsSeen() const;
    qint64 getSessionStartTime() const;
    qint64 getTimeSpent() const;

    // Setters
    void setDeckID(const QString& id);
    void setUserID(const QString& id);

    // Database Operations
    // Load stats from database
    Stats* load() override;
    Stats* loadTotal() override;
    // Initialize stats to database
    bool initialize() const override;

    // Update stats based on user interactions
    bool update(const StatsUpdateContext& context) override;
    // Display stats for debugging
    void display() const override;
};

#endif
