// Card.hpp
#ifndef CARD_H
#define CARD_H

#include <QString>

class Card {
    QString question;
    QString answer;

public:
    Card(QString q, QString a);

    // Getters
    QString getQuestion() const;
    QString getAnswer() const;

    // Setters

    // Database Operations
    bool createCard() const;

    // Comparison operator
    bool operator==(const Card &card) const;
};

#endif