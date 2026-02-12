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
    void setType(CardType type);

    // Database Operations
    bool create() override;
    bool _delete() const override;

    // Display related
    bool update(const StatsUpdateContext &context); // Note: Adjust ease_factor, interval, repetitions (for sm-2 algorithm)

    // Stats
    void getStats() const override;

    // Validator
    bool isEmpty() const;

    // Other
    static CardType stringToType(const QString& str);
    static QString typeToString(CardType type);
    bool saveType() const;
};

#endif
