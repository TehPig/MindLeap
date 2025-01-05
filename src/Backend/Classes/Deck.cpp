#include <utility>
#include <vector>

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

#include "Backend/Classes/Deck.hpp"
#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/generateID.hpp"

// Constructors
Deck::Deck(const QString& name, const std::vector<Card>& c)
    : name(name), cards(c) {}
Deck::Deck(const QString& name, const QString& id) : name(name), id(id) {}
Deck::Deck(const QString& name_or_id) {
    if (name_or_id.startsWith("n_")) this->name = name_or_id.trimmed();
    else this->id = name_or_id.trimmed();
}

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

    // Generate a unique ID
    QString deckId;
    bool idExists = false;

    do {
        deckId = QString::fromStdString(generateID());
        query.prepare(QStringLiteral("SELECT COUNT(*) FROM Decks WHERE id = ?"));
        query.addBindValue(deckId);

        if (!query.exec()) {
            qDebug() << "[DB] Could not check for existing Deck ID: " << query.lastError().text();
            return false;
        }

        query.next();
        idExists = query.value(0).toInt() > 0;
    } while (idExists);

    // Insert the new Deck into the Decks table
    query.prepare(QStringLiteral("INSERT INTO Decks (id, name) VALUES (?, ?)"));
    query.addBindValue(deckId);
    query.addBindValue(this->name.mid(2));

    if (!query.exec()) {
        qDebug() << "[DB] Could not insert Deck into Decks table: " << query.lastError().text();
        return false;
    }

    // Link the Deck to the User in UsersDecks table
    query.prepare(QStringLiteral("INSERT INTO UsersDecks (user_id, deck_id) VALUES (?, ?)"));
    query.addBindValue(userId);
    query.addBindValue(deckId);

    if (!query.exec()) {
        qDebug() << "[DB] Could not link Deck to User in UsersDecks table: " << query.lastError().text();
        return false;
    }

    this->id = deckId;
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

// List all Deck cards
std::vector<Card> Deck::listCards() const {
    std::vector<Card> cards;
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT id, question, answer FROM Cards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
        return cards;
    }

    while (query.next()) {
        cards.emplace_back(query.value("id").toString(), query.value("question").toString(), query.value("answer").toString());
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

// Get Deck stats
void Deck::getStats() const {
    qDebug() << "Deck Stats";
}