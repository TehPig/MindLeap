#include <Backend/Classes/User.hpp>

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

User::User(const std::string &u, int i, const std::vector<Deck> &d) : username(u), id(i), decks(d) {}

// Getters
void User::listDecks() const {
    for (int i = 0; i < this->decks.size(); i++){
        std::cout << i << ". " << decks[i].getName() << " - " << decks[i].getID() << "\n";
    }
}

// Setters
void User::selectUser(int id) {
    this->selected = this;
    std::cout << "Selected User: " << this->username << "\n";
}

// Database Operations
bool User::createUser(std::string username) {

}
// If decks are provided
bool User::createUser(std::string username, std::vector<Deck> deck) {

}

bool User::renameUser(int id, std::string username) {

}

bool User::deleteUser(int id) {

}

void User::addDeck(const Deck &deck){
    decks.push_back(deck);
}

bool User::removeDeck(const Deck &deck){
    const auto item = find(this->decks.begin(), this->decks.end(), deck);
    if (item == this->decks.end()) return false;

    this->decks.erase(item);
    return true;
};