#include <Backend/Classes/User.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

User::User(const std::string &u, int i, const std::vector<Deck> &d) : username(u), id(i), decks(d) {}

void User::listDecks(){
    for (int i = 0; i < this->decks.size(); i++){
        std::cout << i << ". " << decks[i].getName() << " - " << decks[i].getID() << "\n";
    }
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