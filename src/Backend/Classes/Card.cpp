#include <Backend/Classes/Card.h>

#include <string>

Card::Card(const std::string &n, const int id, const std::string &q, const std::string &a) : name(n), id(id), question(q), answer(a) {}

// Getters
std::string Card::getName() const { return this->name; }
int Card::getID() const { return this->id; }
std::string Card::getQuestion(){ return this->question; }
std::string Card::getAnswer(){ return this->answer; }

// Setters

bool Card::operator==(const Card &card) const { return this->id == card.id; }