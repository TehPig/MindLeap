#include <iostream>
#include <string>
#include <Backend/Classes/Card.h>

Card::Card(std::string n, int id, std::string q, std::string a) : name(n), id(id), question(q), answer(a) {}

std::string Card::getName(){ return this->name; }
int Card::getID(){ return this->id; }
std::string Card::getQuestion(){ return this->question; }
std::string Card::getAnswer(){ return this->answer; }