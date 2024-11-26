#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <Backend/Classes/Deck.h>

Deck::Deck(std::string n, int i, std::vector<Card> c) : name(n), id(i), cards(c) {}

void Deck::listCards(){
    for (int i = 0; i < this->cards.size(); i++){
        std::cout << i << ". " << cards[i].getName() << " - " << cards[i].getID() << "\n";
    }
}

void Deck::addCard(const Card card){
    cards.push_back(card);
}

bool Deck::removeCard(const Card card){
    auto item = find(this->cards.begin(), this->cards.end(), card);
    if (item != this->cards.end()) return false;

    this->cards.erase(item);
    return true;
};