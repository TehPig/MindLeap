#include <QString>

#include <Backend/Classes/Deck.hpp>

#include <iostream>
#include <algorithm>
#include <vector>

// Constructor
Deck::Deck(const QString &n, int i, const std::vector<Card> &c)
    : name(n), id(i), cards(c) {}

// Getters
QString Deck::getName() const {
    return this->name;
}

int Deck::getID() const {
    return this->id;
}

void Deck::listCards() const {
    for (size_t i = 0; i < this->cards.size(); ++i) {
        std::cout << i << ". " << cards[i].getName().toStdString()
        << " - " << cards[i].getID() << "\n";
    }
}

// Database Operations

// Add a card to the deck
void Deck::addCard(const Card &card) {
    cards.push_back(card);
}

// Remove a card from the deck
bool Deck::removeCard(const Card &card) {
    auto it = std::find(this->cards.begin(), this->cards.end(), card);
    if (it == this->cards.end()) return false; // Card not found

    this->cards.erase(it);
    return true;
}

// Rename the deck
void Deck::rename(const Deck &deck) {
    if (this->id == deck.getID()) {
        this->name = deck.getName();
        std::cout << "[Deck] Renamed deck to " << this->name.toStdString() << "\n";
    } else {
        std::cerr << "[Deck] Mismatched IDs. Rename failed.\n";
    }
}

// Comparison operator
bool Deck::operator==(const Deck &deck) const {
    return this->id == deck.id;
}
