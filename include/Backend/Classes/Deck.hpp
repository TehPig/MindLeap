#ifndef DECK_H
#define DECK_H

#include <Backend/Classes/Card.hpp>

#include <vector>

class Deck {
private:
    QString name;
    int id;
    std::vector<Card> cards;

public:
    Deck(const QString &n, int i, const std::vector<Card> &c);

    // Getters
    QString getName() const;

    int getID() const;

    void listCards() const;

    // Database Operations
    // Card
    void addCard(const Card &card);
    bool removeCard(const Card &card);

    // Deck
    void rename(const Deck &deck);

    // Comparison
    bool operator==(const Deck &deck) const;
};

#endif
