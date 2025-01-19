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

// Setters
// Set Card Type
void Card::setType(const CardType type) { this->type = type; }

// Database Operations
// Create Card
bool Card::create() {
    logAction("Create Card");

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Generate a unique ID
    QString cardId;
    bool idExists;

    do {
        cardId = generateID();
        qDebug() << "[Debug - Card] Generated ID" << cardId << "for question" << this->question;

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

    this->id = cardId;
    return true;
}

bool Card::_delete() const {
    logAction("Delete Card");

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

bool Card::update(const StatsUpdateContext& context) {
    logAction("Update Card Stats");
    // Update statistics via CardStats
    const bool status = stats.update(context);
    if (status == false) {
        qDebug() << "Failed to update card stats.";
        return false;
    }

    // Log for debugging
    qDebug() << "Card Updated:";
    stats.display();

    return true;
}

// Get Card stats
void Card::getStats() const {
    qDebug() << "Card Stats";
}

// Validator
bool Card::isEmpty() const {
    return this->id.isEmpty() && this->question.isEmpty() && this->answer.isEmpty();
}

// Other
CardType Card::stringToType(const QString& str) {
    if (str == "New") return CardType::New;
    if (str == "Learning") return CardType::Learning;
    if (str == "Review") return CardType::Review;
    return CardType::New;
}
