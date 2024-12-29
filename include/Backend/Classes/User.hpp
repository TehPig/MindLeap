//
// Created by TehPig on 11/27/2024.
//

#ifndef USER_H
#define USER_H

#include <QString>
#include <vector>

#include <Backend/Classes/Deck.hpp>

class User {
    QString username;
    int id;
    std::vector<Deck> decks;
    User *selected;

    // Card Stats
    int times_seen;
    int last_viewed;
    int next_review;

public:
    User(QString u, int i, const std::vector<Deck> &d);

    // Getters
    static std::vector<Deck> listDecks();

    // Setters

    // Database Operations

    // User
    bool createUser() const;
    void selectUser(int id);
    bool renameUser(int id, const QString& username);
    static bool deleteUser(int id);

    // Deck
    void addDeck(const Deck &deck);
    bool removeDeck(const Deck &deck);
};

#endif
