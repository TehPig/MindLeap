// Card.hpp
#ifndef CARD_H
#define CARD_H

#include <QString>

class Card {
    QString name;
    int id;
    QString question;
    QString answer;

public:
    Card(QString n, QString q, QString a);

    // Getters
    QString getName() const;
    int getID() const;
    QString getQuestion() const;
    QString getAnswer() const;

    // Setters
    void setID(int newID);

    // Database Operations
    bool createCard(int deck_id);

    // Comparison operator
    bool operator==(const Card &card) const;
};

#endif