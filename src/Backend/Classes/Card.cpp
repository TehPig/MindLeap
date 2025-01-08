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
Card::Card(const QString& id, const QString& q, const QString& a, const CardType& type)
    : id(id), question(q), answer(a), type(type) {}

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

// Get Card Type
CardType Card::getType() const { return this->type; }

// Setters
// Set which algorithm to use
void Card::setSM2(const bool state) {
    this->useSM2 = state;
}

// Set Card Type
void Card::setType(const CardType type) { this->type = type; }

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
    const StatsUpdateContext context = {
        quality,               // Button pressed (1-4)
        30,                    // Time spent (example value)
        QDateTime::currentDateTime()
    };

    this->stats.times_seen++;
    this->stats.time_spent_seconds += context.time_spent;
    this->stats.last_seen = QDateTime::currentDateTime();

    if (useSM2) {
        // SM-2 Algorithm logic
        repetitions = (quality < 3) ? 0 : repetitions + 1;

        if (quality == 1) { // "Again"
            interval = 1;
        } else {
            interval = (repetitions == 1) ? 1 : interval * easeFactor;
            easeFactor += 0.1 - (4 - quality) * (0.08 + (4 - quality) * 0.02);
            if (easeFactor < 1.3) easeFactor = 1.3;
        }
    } else {
        // Leitner System logic
        switch (quality) {
            case 1: interval = 0; break; // Immediate retry
            case 2: interval = 1; break; // 1 day
            case 3: interval = 3; break; // 3 days
            case 4: interval = 7; break; // 7 days
            default: interval = 1; break;
        }
    }

    stats.time_to_reappear = interval;
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

// Other
CardType Card::stringToType(const QString& str) {
    if (str == "New") return CardType::New;
    if (str == "Learning") return CardType::Learning;
    if (str == "Review") return CardType::Review;
    return CardType::New;
}