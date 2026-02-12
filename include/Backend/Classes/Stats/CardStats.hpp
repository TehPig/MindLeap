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
    qint64 last_seen;
    float easeFactor = 2.5; // default ease factor value for sm-2
    int interval;
    int repetitions;
    qint64 card_start_time;

public:
    // Constructors
    CardStats(
        const QString& card_id,
        const QString& user_id,
        const QDate& date,
        const int& times_seen,
        const int& time_spent_seconds,
        const qint64& last_seen,
        const float& easeFactor,
        const int& interval,
        const int& repetitions,
        const qint64& card_start_time);
    CardStats(const QString& card_id, const QString& user_id);
    CardStats();

    // Getters
    QString getCardID() const;
    QString getUserID() const;
    int getTimesSeen() const;
    int getTimeSpent() const;
    qint64 getCardStartTime() const;
    qint64 getLastSeen() const;
    float getEaseFactor() const;
    int getInterval() const;
    int getRepetitions() const;

    // Setters
    void setCardID(const QString &card_id);
    void setUserID(const QString &user_id);

    void setEaseFactor(float easeFactor);
    void setInterval(int interval);
    void setRepetitions(int repetitions);

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
