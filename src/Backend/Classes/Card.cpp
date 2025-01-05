// Card.cpp
#include <sstream>
#include <random>

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

#include "Backend/Utilities/generateID.hpp"
#include "Backend/Classes/Card.hpp"
#include "Backend/Database/setup.hpp"

// Constructors
Card::Card(const QString& id, const QString& q, const QString& a)
    : id(id), question(q), answer(a)
{
}

Card::Card(const QString& q, const QString& a)
: question(q), answer(a) {}
Card::Card(const QString& id)
    : id(id) {}

// Getters
// Get Card ID
QString Card::getID() const { return this->id; }

// Get Card Question
QString Card::getQuestion() const { return this->question; }

// Get Card Answer
QString Card::getAnswer() const { return this->answer; }

// Setters


// Database Operations
// Create Card
bool Card::create() {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Generate a unique ID
    QString cardId;
    bool idExists;

    do {
        cardId = QString::fromStdString(generateID());
        qDebug() << "Card >> Generated ID: " << cardId;

        query.prepare("INSERT INTO Cards (id, question, answer) VALUES (?, ?, ?);");
        query.addBindValue(cardId);
        query.addBindValue(this->question);
        query.addBindValue(this->answer);

        if (!query.exec()) {
            qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
            return false;
        }

        idExists = query.numRowsAffected() == 0;
    } while (idExists);

    this->id = cardId;
    qDebug() << "Card created successfully with ID: " << cardId;
    return true;
}

bool Card::_delete() const {
    if (this->id.isEmpty()) {
        qDebug() << "[DB] Card Delete - Missing ID.";
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("DELETE FROM Cards WHERE id = ?;");
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to delete card: " << query.lastError().text();
        return false;
    }

    return true;
}

void Card::updateCard(const int quality) {
    // Run code here
}

// Calculate Card next interval
int Card::calculateNextInterval(const int buttonPressed, const int currentInterval) {
    switch (buttonPressed) {
        case 1: return currentInterval * 0.5; // Again
        case 2: return currentInterval * 1.2; // Hard
        case 3: return currentInterval * 2.0; // Good
        case 4: return currentInterval * 3.0; // Easy
        default: return currentInterval;
    }
}

// Get Card stats
void Card::getStats() const {
    qDebug() << "Card Stats";
}