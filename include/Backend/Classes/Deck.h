#ifndef DECK_H
#define DECK_h

#include <string>
#include <algorithm>
#include <vector>
#include <Backend/Classes/Card.h>

class Deck {
    private:
        std::string name;
        int id;
        std::vector<Card> cards;

    public:
        Deck(std::string n, int i, std::vector<Card> c) : name(n), id(i), cards(c) {}

        void listCards();
        void addCard(const Card card);
        bool removeCard(const Card card);
};

#endif