#ifndef DECK_H
#define DECK_H

#include <vector>
#include <QString>

#include <Backend/Classes/Card.hpp>

class Deck {
    QString name;
    int id;
    std::vector<Card> cards;

public:
    Deck(QString n, int i, const std::vector<Card> &c);
    Deck(QString n, int i);

    // Getters
    QString getName() const;

    int getID() const;

    std::vector<Card> listCards() const;
    int getCardCount() const;

    // Database Operations

    // Card
    bool addCard(Card &card);
    bool removeCard(const Card &card);

    // Deck
    void rename(const QString &newName);

    // Comparison
    bool operator==(const Deck &deck) const;
};

#endif
