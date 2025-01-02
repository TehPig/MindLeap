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
    QString id;
    std::vector<Deck> decks;

public:
    // Constructors
    //User(const QString& u, QString& i, const std::vector<Deck> &d);
    User(const QString& name, const QString& id);
    User(const QString& name_or_id);
    // For the user of listUsers function
    User();

    // Getters
    QString getUsername() const;
    QString getID() const;

    // Setters

    // Database Operations
    // User
    bool create();
    bool select();
    bool rename(const QString& username);
    bool _delete() const;

    static std::vector<User> listUsers();

    // Deck
    void addDeck(const Deck &deck);
    bool removeDeck(const Deck &deck);

    std::vector<Deck> listDecks();
};

#endif
