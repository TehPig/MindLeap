#include <utility>
#include <vector>
#include <iostream>
#include <algorithm>

#include <QSqlQuery>
#include <QSqlError>

#include "Backend/Classes/Deck.hpp"
#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/generateID.hpp"

// Constructor
Deck::Deck(const QString& name, const std::vector<Card>& c)
    : name(name), cards(c) {}
Deck::Deck(const QString& name, const QString& id) : name(name), id(id) {}
Deck::Deck(const QString& name_or_id) {
    if (name_or_id.startsWith("n_")) this->name = name_or_id;
    else this->id = name_or_id;
}

// Getters
QString Deck::getName() const { return this->name; }

QString Deck::getID() const { return this->id; }

// Database Operations
std::vector<Card> Deck::listCards() const {
    std::vector<Card> cards;
    const Database* db = Database::getInstance();
    QSqlQuery sqlQuery(db->getDB());

    sqlQuery.prepare("SELECT id, question, answer FROM Cards WHERE deck_id = ?");
    sqlQuery.addBindValue(this->id);

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Failed to execute query: " << sqlQuery.lastError().text().toStdString() << "\n";
        return cards;
    }

    while (sqlQuery.next()) {
        QString id = sqlQuery.value(0).toString();
        QString question = sqlQuery.value(1).toString();
        QString answer = sqlQuery.value(2).toString();

        cards.emplace_back(id, question, answer);
    }

    return cards;
}

int Deck::getCardCount() const {
    const Database* db = Database::getInstance();
    QSqlQuery sqlQuery(db->getDB());

    sqlQuery.prepare("SELECT COUNT(*) FROM DecksCards WHERE deck_id = ?");
    sqlQuery.addBindValue(this->id);

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Failed to execute query: " << sqlQuery.lastError().text().toStdString() << "\n";
        return -1; // If query fails
    }

    if (!sqlQuery.next()) return 0; // Return 0 if no cards are stored
    return sqlQuery.value(0).toInt();
}

bool Deck::create() const {
    const Database* db = Database::getInstance();
    QSqlQuery sqlQuery(db->getDB());

    // Retrieve saved user ID
    sqlQuery.prepare("SELECT id FROM SavedUser LIMIT 1");
    if (!sqlQuery.exec() || !sqlQuery.next()) {
        std::cerr << "[DB] Could not retrieve user ID - create Deck: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    const QString userId = sqlQuery.value(0).toString();

    // Generate a unique ID
    QString deckId;
    bool idExists = false;

    do {
        deckId = QString::fromStdString(generateID());
        sqlQuery.prepare("SELECT COUNT(*) FROM Decks WHERE id = ?");
        sqlQuery.addBindValue(deckId);

        if (!sqlQuery.exec()) {
            std::cerr << "[DB] Could not check for existing Deck ID: " << sqlQuery.lastError().text().toStdString() << "\n";
            return false;
        }

        sqlQuery.next();
        idExists = sqlQuery.value(0).toInt() > 0;
    } while (idExists);

    // Insert the new Deck into the Decks table
    sqlQuery.prepare("INSERT INTO Decks (id, name) VALUES (?, ?)");
    sqlQuery.addBindValue(deckId);
    sqlQuery.addBindValue(this->name.mid(2));

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Could not insert Deck into Decks table: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    // Link the Deck to the User in UsersDecks table
    sqlQuery.prepare("INSERT INTO UsersDecks (user_id, deck_id) VALUES (?, ?)");
    sqlQuery.addBindValue(userId);
    sqlQuery.addBindValue(deckId);

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Could not link Deck to User in UsersDecks table: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    return true;
}

bool Deck::setDescription(const QString& description) const {
    const Database* db = Database::getInstance();
    QSqlQuery sqlQuery(db->getDB());

    sqlQuery.prepare("UPDATE Decks SET description = ? WHERE id = ?");
    sqlQuery.addBindValue(description);
    sqlQuery.addBindValue(this->id);

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Could not update Deck description: " << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    return true;
}

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
void Deck::rename(const QString& newName) {
    const Database* db = Database::getInstance();
    const QString query = "UPDATE Decks SET name = ? WHERE id = ?;";

    QSqlQuery sqlQuery(db->getDB());
    if (!sqlQuery.prepare(query))
    {
        std::cerr << "[DB] Failed to prepare query: "
            << sqlQuery.lastError().text().toStdString() << "\n";
        return;
    }

    sqlQuery.addBindValue(newName);
    sqlQuery.addBindValue(this->id);

    if (!sqlQuery.exec())
    {
        std::cerr << "[DB] Failed to execute query: "
            << sqlQuery.lastError().text().toStdString() << "\n";
        return;
    }

    this->name = newName;
    std::cout << "[Deck] Renamed deck to " << this->name.toStdString() << "\n";
}
