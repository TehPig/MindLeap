// Card.hpp
#ifndef CARD_H
#define CARD_H

#include <QString>

class Card
{
    QString id;
    QString question;
    QString answer;

public:
    // Constructors
    Card(const QString& id, const QString& q, const QString& a);
    Card(const QString& q, const QString& a);
    Card(const QString& id);

    // Getters
    QString getID() const;
    QString getQuestion() const;
    QString getAnswer() const;

    // Setters
    void setID(const QString& id);

    // Database Operations
    bool createCard() const;
};

#endif
