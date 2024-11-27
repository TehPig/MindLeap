//
// Created by TehPig on 11/27/2024.
//

#ifndef USER_H
#define USER_H

#include <Backend/Classes/Deck.h>

#include <string>
#include <vector>

class User {
    private:
        std::string username;
        int id;
        std::vector<Deck> decks;

    public:
        User(const std::string &u, int i, const std::vector<Deck> &d);

        void listDecks();
        void addDeck(const Deck &deck);
        bool removeDeck(const Deck &deck);
};

#endif //USER_H
