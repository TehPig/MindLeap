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
    : id(id), question(q), answer(a), type(type), stats({}) {}

Card::Card(const QString& q, const QString& a)
: question(q), answer(a) {}
Card::Card(const QString& id)
    : id(id) {}
Card::Card() = default;

// Getters
// Get Card ID
QString Card::getID() const { return this->id; }

// Get Card Question
QString Card::getQuestion() const { return this->question; }

// Get Card Answer
QString Card::getAnswer() const { return this->answer; }

// Get Card Type
CardType Card::getType() const { return this->type; }

// Get Repetitions
int Card::getRepetitions() const { return repetitions; }

// Get ease factor
double Card::getEaseFactor() const { return easeFactor; }

// Get interval
double Card::getInterval() const { return interval; }

// Setters
// Set which algorithm to use
void Card::setSM2(const bool state) { this->useSM2 = state; }

// Set Card Type
void Card::setType(const CardType type) { this->type = type; }

// Set Repetitions
void Card::setRepetitions(const int value) { repetitions = value; }

// Set ease factor
void Card::setEaseFactor(const double value) { easeFactor = value; }

// Set interval
void Card::setInterval(const double value) { interval = value; }

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
        qDebug() << "[Debug - Card] Generated ID: " << cardId << " for question " << this->question;

        query.prepare("INSERT INTO Cards (id, question, answer) VALUES (?, ?, ?);");
        query.addBindValue(cardId);
        query.addBindValue(this->question);
        query.addBindValue(this->answer);

        if (!query.exec()) {
            if (query.lastError().nativeErrorCode() == QStringLiteral("19")) {
                qDebug() << "[DB] ID already exists, generating a new one.";
                idExists = true;
            } else {
                qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
                return false;
            }
        } else idExists = false;
    } while (idExists);

    qDebug() << "Card created successfully with ID: " << cardId;
    this->id = cardId;
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

void Card::update(const StatsUpdateContext& context) {
    // Update statistics via CardStats
    stats.updateStats(context);

    // Ensure stats reflect the updated interval
    stats.setTimeToReappear(interval);

    if (interval > 1.0) {
        qDebug() << "Card skipped for today (interval > 1 day).";
        return;
    }

    // Log for debugging
    qDebug() << "Card Updated:";
    stats.displayStats();
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