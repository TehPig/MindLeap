//
// Created by TehPig on 11/27/2024.
//

#ifndef USER_H
#define USER_H

#include <QString>
#include <vector>

#include "Backend/Classes/Base/Entity.hpp"
#include "Backend/Classes/Deck.hpp"
#include "Backend/Classes/Stats/UserStats.hpp"

class User final : public Entity {
private:
    QString username;
    QString id;
    std::vector<Deck> decks;
    //UserStats stats;

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
    bool create() override;
    bool _delete() const override;

    bool select() const;
    bool rename(const QString& username);

    static std::vector<User> listUsers();
    std::vector<Deck> listDecks() const;

    // Stats
    void getStats() const override;
};

#endif
