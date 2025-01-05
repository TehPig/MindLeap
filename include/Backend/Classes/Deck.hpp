#ifndef DECK_H
#define DECK_H

#include <vector>
#include <QString>

#include "Backend/Classes/Base/Entity.hpp"
#include "Backend/Classes/Card.hpp"
#include "Backend/Classes/Stats/DeckStats.hpp"

class Deck final : public Entity {
private:
    QString name;
    QString id;
    std::vector<Card> cards;
    //DeckStats stats;

public:
    // Constructors
    Deck(const QString& name, const std::vector<Card>& c);
    Deck(const QString& name, const QString& id);
    Deck(const QString& name_or_id);

    // Getters
    QString getName() const;
    QString getID() const;

    // Database Operations
    // Deck
    bool create() override;
    bool _delete() const override;

    bool rename(const QString& newName);
    bool setDescription(const QString& description) const;

    std::vector<Card> listCards() const;
    int getCardCount() const;

    // Stats
    void getStats() const override;
};

#endif
