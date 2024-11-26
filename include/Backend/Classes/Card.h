#ifndef CARD_H
#define CARD_H

#include <string>

class Card {
    protected:
        std::string name;
        int id;
        std::string question;
        std::string answer;

    public:
        Card(std::string n, int id, std::string q, std::string a);

        // Getters
        std::string getName();
        int getID();
        std::string getQuestion();
        std::string getAnswer();
};

#endif