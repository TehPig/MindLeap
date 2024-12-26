#ifndef CARD_H
#define CARD_H

#include <string>

class Card {
    private:
        std::string name;
        int id;
        std::string question;
        std::string answer;

    public:
        Card(const std::string &n, int id, const std::string &q, const std::string &a);

        // Getters
        std::string getName() const;
        int getID() const;
        std::string getQuestion();
        std::string getAnswer();

        // Database Operations
        bool createCard(const int deck_id) const;

        // Comparison
        bool operator==(const Card &card) const;
};

#endif