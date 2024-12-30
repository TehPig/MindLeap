#include <utility>
#include <vector>
#include <iostream>
#include <algorithm>

#include <QSqlQuery>
#include <QSqlError>

#include "Backend/Classes/Deck.hpp"
#include "Backend/Database/setup.hpp"

// Constructor
Deck::Deck(QString n, const int i, const std::vector<Card> &c)
    : name(std::move(n)), id(i), cards(c) {}

Deck::Deck(QString n, const int i)
    : name(std::move(n)), id(i) {}

// Getters
QString Deck::getName() const {
    return this->name;
}

int Deck::getID() const {
    return this->id;
}

std::vector<Card> Deck::listCards() const {
    std::vector<Card> cards;
    const Database *db = Database::getInstance();
    QSqlQuery sqlQuery(db->getDB());

    sqlQuery.prepare("SELECT id, name, question, answer FROM Cards WHERE deck_id = ?");
    sqlQuery.addBindValue(this->id);

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Failed to execute query: "
                  << sqlQuery.lastError().text().toStdString() << "\n";
        return cards;
    }

    while (sqlQuery.next()) {
        int id = sqlQuery.value(0).toInt();
        QString name = sqlQuery.value(1).toString();
        QString question = sqlQuery.value(2).toString();
        QString answer = sqlQuery.value(3).toString();
        cards.emplace_back(question, answer);
        //cards.back().setID(id); // Set the ID for the card
    }

    return cards;
}

int Deck::getCardCount() const {
    const Database *db = Database::getInstance();
    QSqlQuery sqlQuery(db->getDB());

    sqlQuery.prepare("SELECT COUNT(*) FROM Cards WHERE deck_id = ?");
    sqlQuery.addBindValue(this->id);

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Failed to execute query: "
                  << sqlQuery.lastError().text().toStdString() << "\n";
        return 0;
    }

    if (sqlQuery.next()) {
        return sqlQuery.value(0).toInt();
    }

    return 0;
}

// Database Operations

// Add a card to the deck
// bool Deck::addCard(Card &card) {
//     const Database *db = Database::getInstance();
//
//     // Insert the card into the Cards table
//     QSqlQuery sqlQuery(db->getDB());
//     sqlQuery.prepare("INSERT INTO Cards (id, question, answer) VALUES (?, ?, ?);");
//     sqlQuery.addBindValue(this->id);
//     sqlQuery.addBindValue(card.getQuestion());
//     sqlQuery.addBindValue(card.getAnswer());
//
//     if (!sqlQuery.exec()) {
//         std::cerr << "[DB] Failed to execute query: "
//                   << sqlQuery.lastError().text().toStdString() << "\n";
//         return false;
//     }
//
//     // Insert the deck_id and card_id into the deckscards table
//     const QString query = "INSERT INTO deckscards (deck_id, card_id) VALUES (?, ?);";
//     if (!sqlQuery.prepare(query)) {
//         std::cerr << "[DB] Failed to prepare query: "
//                   << sqlQuery.lastError().text().toStdString() << "\n";
//         return false;
//     }
//
//     sqlQuery.addBindValue(this->id);
//     //sqlQuery.addBindValue(cardID);
//
//     if (!sqlQuery.exec()) {
//         std::cerr << "[DB] Failed to execute query: "
//                   << sqlQuery.lastError().text().toStdString() << "\n";
//         return false;
//     }
//
//     // Add the card to the deck's card list
//     cards.push_back(card);
//     return true;
// }

// Remove a card from the deck
// bool Deck::removeCard(const Card &card) {
//     const Database *db = Database::getInstance();
//     const QString query = "DELETE FROM deckscards WHERE card_id = ? AND deck_id = ?;";
//
//     QSqlQuery sqlQuery(db->getDB());
//     if (!sqlQuery.prepare(query)) {
//         std::cerr << "[DB] Failed to prepare query: "
//                   << sqlQuery.lastError().text().toStdString() << "\n";
//         return false;
//     }
//
//     //sqlQuery.addBindValue(card.getID());
//     //sqlQuery.addBindValue(this->id);
//
//     if (!sqlQuery.exec()) {
//         std::cerr << "[DB] Failed to execute query: "
//                   << sqlQuery.lastError().text().toStdString() << "\n";
//         return false;
//     }
//
//     const auto it = std::find_if(this->cards.begin(), this->cards.end(),
//                                  [&card](const Card &c) { return c == card; });
//     if (it == this->cards.end()) {
//         std::cerr << "[Deck] Card not found. Remove failed.\n";
//         return false;
//     }
//
//     this->cards.erase(it);
//     return true;
// }

// Rename the deck
void Deck::rename(const QString &newName) {
    const Database *db = Database::getInstance();
    const QString query = "UPDATE Decks SET name = ? WHERE id = ?;";

    QSqlQuery sqlQuery(db->getDB());
    if (!sqlQuery.prepare(query)) {
        std::cerr << "[DB] Failed to prepare query: "
                  << sqlQuery.lastError().text().toStdString() << "\n";
        return;
    }

    sqlQuery.addBindValue(newName);
    sqlQuery.addBindValue(this->id);

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Failed to execute query: "
                  << sqlQuery.lastError().text().toStdString() << "\n";
        return;
    }

    this->name = newName;
    std::cout << "[Deck] Renamed deck to " << this->name.toStdString() << "\n";
}

// Comparison operator
bool Deck::operator==(const Deck &deck) const {
    return this->id == deck.id;
}
