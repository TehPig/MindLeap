#include <Backend/Classes/Card.hpp>
#include <Backend/Database/setup.hpp>

#include <string>

Card::Card(const std::string &n, const int id, const std::string &q, const std::string &a) : name(n), id(id), question(q), answer(a) {}

// Getters
std::string Card::getName() const { return this->name; }
int Card::getID() const { return this->id; }
std::string Card::getQuestion(){ return this->question; }
std::string Card::getAnswer(){ return this->answer; }

// Database operations
bool Card::createCard(const int deck_id) const {
    Database *db = Database::getInstance("app_data.db");
    std::string query = "INSERT INTO Cards (question, answer, deck_id) VALUES (?, ?, ?);";

    if (!db->prepare(query, {this->question, this->answer}, {deck_id})) return false;
    return true;
};

bool Card::operator==(const Card &card) const { return this->id == card.id; }