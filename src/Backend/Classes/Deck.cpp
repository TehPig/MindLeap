#include <Backend/Classes/Deck.hpp>

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

Deck::Deck(const std::string &n, int i, const std::vector<Card> &c) : name(n), id(i), cards(c) {}

// Getters
std::string Deck::getName() const {
    return this->name;
}

int Deck::getID() const {
    return this->id;
}

void Deck::listCards() const {
    for (int i = 0; i < this->cards.size(); i++){
        std::cout << i << ". " << cards[i].getName() << " - " << cards[i].getID() << "\n";
    }
}

// Database Operations
void Deck::addCard(const Card &card){
    cards.push_back(card);
}

bool Deck::removeCard(const Card &card){
    const auto item = find(this->cards.begin(), this->cards.end(), card);
    if (item == this->cards.end()) return false;

    this->cards.erase(item);
    return true;
};

bool Deck::operator==(const Deck &deck) const { return this->id == deck.id; }