#ifndef CARD_H
#define CARD_H

#include <QString>

class Card {
    private:
        QString name;
        int id;
        QString question;
        QString answer;

    public:
        Card(const QString &n, int id, const QString &q, const QString &a);

        // Getters
        QString getName() const;
        int getID() const;
        QString getQuestion() const;
        QString getAnswer() const;

        // Database Operations
        bool createCard(const int deck_id) const;

        // Comparison
        bool operator==(const Card &card) const;
};

#endif
