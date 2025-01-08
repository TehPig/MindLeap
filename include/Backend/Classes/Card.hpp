// Card.hpp
#ifndef CARD_H
#define CARD_H

#include <QString>

#include "Backend/Classes/Base/Entity.hpp"
#include "Backend/Classes/Stats/CardStats.hpp"

enum class CardType {
    New,
    Learning,
    Review
};

class Card final : public Entity {
private:
    QString id;
    QString question;
    QString answer;
    float easeFactor = 2.5;
    int interval = 1;
    int repetitions = 0;
    bool useSM2 = false;
    CardType type = CardType::New;

    CardStats stats;

public:
    // Constructors
    Card(const QString& id, const QString& q, const QString& a, const CardType& type);
    Card(const QString& q, const QString& a);
    Card(const QString& id);
    Card();

    // Getters
    QString getID() const;
    QString getQuestion() const;
    QString getAnswer() const;

    CardType getType() const;

    // Setters
    void setSM2(bool state);
    void setType(CardType type);

    // Database Operations
    bool create() override;
    bool _delete() const override;

    // Display related
    void updateCard(int quality); // Note: Adjust ease_factor, interval, repetitions (for sm-2 algorithm)
    static int calculateNextInterval(int buttonPressed, int currentInterval);

    // Stats
    void getStats() const override;

    // Other
    static CardType stringToType(const QString& str);
};

#endif
