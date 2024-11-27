//
// Created by TehPig on 11/27/2024.
//

#ifndef USER_H
#define USER_H

#include <Backend/Classes/Deck.hpp>

#include <string>
#include <vector>

class User {
    private:
        std::string username;
        int id;
        std::vector<Deck> decks;
        User *selected;

    public:
        User(const std::string &u, int i, const std::vector<Deck> &d);

        // Getters
        void listDecks() const;

        // Setters
        void selectUser(int id);

        // Database Operations
        bool createUser(std::string username);
        bool createUser(std::string username, std::vector<Deck> decks);

        bool renameUser(int id, std::string username);
        bool deleteUser(int id);

        void addDeck(const Deck &deck);
        bool removeDeck(const Deck &deck);
};

#endif
