// Card.hpp
#ifndef CARD_H
#define CARD_H

#include <QString>

#include "Backend/Classes/Base/Entity.hpp"
#include "Backend/Classes/Stats/CardStats.hpp"

class Card final : public Entity {
private:
    QString id;
    QString question;
    QString answer;
    float easeFactor = 2.5;
    int interval = 1;
    int repetitions = 0;
    //CardStats stats;

public:
    // Constructors
    Card(const QString& id, const QString& q, const QString& a);
    Card(const QString& q, const QString& a);
    Card(const QString& id);

    // Getters
    QString getID() const;
    QString getQuestion() const;
    QString getAnswer() const;

    // Setters

    // Database Operations
    bool create() override;
    bool _delete() const override;

    // Display related
    static void updateCard(int quality); // Note: Adjust ease_factor, interval, repetitions (for sm-2 algorithm)
    static int calculateNextInterval(int buttonPressed, int currentInterval);

    // Stats
    void getStats() const override;
};

#endif
