#include <Backend/Classes/Card.hpp>
#include <Backend/Database/setup.hpp>

#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <iostream>

// Constructor
Card::Card(const QString &n, const int id, const QString &q, const QString &a)
    : name(n), id(id), question(q), answer(a) {}

// Getters
QString Card::getName() const {
    return this->name;
}

int Card::getID() const {
    return this->id;
}

QString Card::getQuestion() const {
    return this->question;
}

QString Card::getAnswer() const {
    return this->answer;
}

// Database Operations
bool Card::createCard(const int deck_id) const {
    Database *db = Database::getInstance();
    QString query = "INSERT INTO Cards (question, answer, deck_id) VALUES (?, ?, ?);";

    QSqlQuery sqlQuery(db->getDB());
    if (!sqlQuery.prepare(query)) {
        std::cerr << "[DB] Failed to prepare query: "
                  << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    sqlQuery.addBindValue(this->question);
    sqlQuery.addBindValue(this->answer);
    sqlQuery.addBindValue(deck_id);

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Failed to execute query: "
                  << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    return true;
}

// Comparison operator
bool Card::operator==(const Card &card) const {
    return this->id == card.id;
}
