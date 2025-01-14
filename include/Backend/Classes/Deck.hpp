#ifndef DECK_H
#define DECK_H

#include <vector>
#include <queue>

#include <QString>

#include "Backend/Classes/Base/Entity.hpp"
#include "Backend/Classes/Card.hpp"
#include "Backend/Classes/Stats/DeckStats.hpp"

class Deck final : public Entity {
private:
    QString name;
    QString id;
    std::queue<Card> studyQueue;

    DeckStats stats;

public:
    // Constructors
    Deck(const QString& name, const std::queue<Card>& c);
    Deck(const QString& name, const QString& id);
    Deck(const QString& name_or_id);
    Deck();

    // Getters
    QString getName() const;
    QString getID() const;

    // Database Operations
    // Deck
    bool create() override; // add ability to create default deck
    bool _delete() const override;

    Deck fetch() const;

    bool addCard(Card& card) const;

    //bool find() const; // Find a deck by name or ID

    bool rename(const QString& newName);
    bool setDescription(const QString& description) const;
    QString getDescription() const;

    std::vector<Card> listCards() const;
    int getCardCount() const;

    // Studying
    bool study();
    bool processCardResponse(Card& card, int buttonPressed);
    bool endStudy() const;

    Card getNextCard();

    // Stats
    void getStats() const override;
};

#endif
