#ifndef DECK_H
#define DECK_H

#include <vector>
#include <QString>

#include <Backend/Classes/Card.hpp>

class Deck
{
    QString name;
    QString id;
    std::vector<Card> cards;

public:
    // Constructors
    Deck(const QString& name, const std::vector<Card>& c);
    Deck(const QString& name, const QString& id);
    Deck(const QString& name_or_id);

    // Getters
    QString getName() const;
    QString getID() const;

    // Database Operations
    // Card
    std::vector<Card> listCards() const;
    int getCardCount() const;

    bool addCard(Card& card) const;
    bool removeCard(const Card& card) const;

    bool setDescription(const QString& description) const;

    // Deck
    bool create() const; // Get the deck name from the constructor
    void rename(const QString& newName);
};

#endif
