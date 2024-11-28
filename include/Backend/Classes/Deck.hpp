#ifndef DECK_H
#define DECK_H

#include <Backend/Classes/Card.hpp>

#include <string>
#include <vector>

class Deck {
private:
    std::string name;
    int id;
    std::vector<Card> cards;

public:
    Deck(const std::string &n, int i, const std::vector<Card> &c);

    // Getters
    std::string getName() const;

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
