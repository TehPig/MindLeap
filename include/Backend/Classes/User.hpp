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

    // Card Stats
    int times_seen;
    int last_viewed;
    int next_review;

public:
    User(const std::string &u, int i, const std::vector<Deck> &d);

    // Getters
    void listDecks() const;

    // Setters

    // Database Operations
    // User
    bool createUser() const;

    void selectUser(int id);

    bool renameUser(int id, std::string username);

    bool deleteUser(int id);

    // Deck
    void addDeck(const Deck &deck);

    bool removeDeck(const Deck &deck);
};

#endif
