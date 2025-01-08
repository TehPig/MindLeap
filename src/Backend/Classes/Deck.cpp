#include <utility>
#include <vector>

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

#include "Backend/Classes/Deck.hpp"
#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/createUniqueDeck.hpp"

// Constructors
Deck::Deck(const QString& name, const std::vector<Card>& c)
    : name(name), cards(c) {}
Deck::Deck(const QString& name, const QString& id) :
    name(name), id(id) {}
Deck::Deck(const QString& name_or_id) {
    if (name_or_id.startsWith("n_")) this->name = name_or_id.trimmed();
    else this->id = name_or_id.trimmed();
}
// For returning results
Deck::Deck() {}

// Getters
// Get Deck name
QString Deck::getName() const {
    return this->name.startsWith("n_") ? this->name.mid(2) : this->name;
}

// Get Deck ID
QString Deck::getID() const { return this->id; }

// Setters


// Database Operations
// Create Deck
bool Deck::create() {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Retrieve saved user ID
    query.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Could not retrieve user ID - create Deck: " << query.lastError().text();
        return false;
    }

    const QString userId = query.value("id").toString();

    if (this->name.isEmpty()) {
        // Check if Default user exists
        query.prepare(QStringLiteral("SELECT COUNT(*) FROM Decks WHERE name = 'Default';"));
        if (!query.exec()) {
            qDebug() << "[DB] Failed to check for default user: " << query.lastError().text();
            return false;
        }

        if (query.next() && query.value(0).toInt() > 0) {
            qDebug() << "[DB] Default user already exists. For a new user, specify a username.";
            return false;
        }

        return createUniqueDeck("Default");
    }

    return createUniqueDeck(this->name);
}

// Delete Deck
bool Deck::_delete() const {
    if (this->id.isEmpty()) {
        qDebug() << "[DB] Deck Delete - Missing ID.";
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("DELETE FROM Decks WHERE id = ?;");
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to delete deck: " << query.lastError().text();
        return false;
    }

    return true; // DecksCards and DeckStats are automatically cleaned up by cascading rules.
}

// Fetch Deck from database
Deck Deck::fetch() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    if (!this->id.isEmpty()) {
        query.prepare(QStringLiteral("SELECT * FROM Decks WHERE id = ?;"));
        query.addBindValue(this->id);
    } else if (!this->name.isEmpty()) {
        query.prepare(QStringLiteral("SELECT * FROM Decks WHERE name = ?;"));
        query.addBindValue(this->name);
    } else {
        qDebug() << "[DB] Deck Fetch - Unable to fetch without ID or username.";
        return Deck();
    }

    return Deck(query.value("name").toString(), query.value("id").toString());
}

// Add Card to Deck
bool Deck::addCard(Card& card) const {
    // Check if the Card exists in the Cards table
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM Cards WHERE id = ? LIMIT 1"));
    query.addBindValue(card.getID());

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Card not found in Cards table, creating Card...";
        if (!card.create()) {
            qDebug() << "[DB] Could not create Card.";
            return false;
        }
    }

    query.prepare(QStringLiteral("INSERT INTO DecksCards (deck_id, card_id) VALUES (?, ?)"));
    query.addBindValue(this->id);
    query.addBindValue(card.getID());

    if (!query.exec()) {
        qDebug() << "[DB] Could not link Card and Deck: " << query.lastError().text();
        return false;
    }

    return true;
}

// Rename Deck
bool Deck::rename(const QString& newName) {
    if (newName.trimmed().isEmpty()) {
        qDebug() << "[DB] Deck Rename - Invalid name specified.";
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("UPDATE Decks SET name = ? WHERE id = ?;"));
    query.addBindValue(newName);
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
        return false;
    }

    this->name = newName;
    qDebug() << "[Deck] Renamed deck to " << this->name;

    return true;
}

// Set Deck description
bool Deck::setDescription(const QString& description) const {
    if (description.trimmed().isEmpty()) {
        qDebug() << "[DB] Deck Set Description - Missing description.";
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("UPDATE Decks SET description = ? WHERE id = ?"));
    query.addBindValue(description);
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Could not update Deck description: " << query.lastError().text();
        return false;
    }

    return true;
}

// Get Deck description
QString Deck::getDescription() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT description FROM Decks WHERE id = ?"));
    query.addBindValue(this->id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Could not retrieve Deck description: " << query.lastError().text();
        return QString();
    }

    return query.value("description").toString();
}

// List all Deck cards
std::vector<Card> Deck::listCards() const {
    std::vector<Card> cards;
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT id, question, answer, type FROM Cards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
        return cards;
    }

    while (query.next()) {
        studyQueue.push(Card(query.value("id").toString(), query.value("question").toString(), query.value("answer").toString(), Card::stringToType(query.value("type").toString())));
    }

    return cards;
}

// Get card count in the deck
int Deck::getCardCount() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT COUNT(*) FROM DecksCards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
        return -1;
    }

    return query.next() ? query.value(0).toInt() : 0;
}

// Studying
bool Deck::study() {
    if (this->id.isEmpty()) {
        qDebug() << "[DB] Deck Study - Missing ID.";
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    qDebug() << "[Info] Studying Deck: " << this->name;

    query.prepare(QStringLiteral("SELECT card_id FROM DecksCards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Could retrieve cards for studying: " << query.lastError().text();
        return false;
    }

    while (query.next()) {

    }
}

// Get Next Card
Card Deck::getNextCard() {
    if (this->studyQueue.empty()) {
        qDebug() << "[Info] Study Queue is empty.";
        return Card();
    }

    const Card nextCard = this->studyQueue.front();
    this->studyQueue.pop();
    return nextCard;
}

// Get Deck stats
void Deck::getStats() const {
    qDebug() << "Deck Stats";
}