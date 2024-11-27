#ifndef DECK_H
#define DECK_H

#include <Backend/Classes/Card.h>

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
        std::string getName();
        int getID() const;
        void listCards();

        void addCard(const Card &card);
        bool removeCard(const Card &card);
        bool operator==(const Deck &deck) const;
};

#endif