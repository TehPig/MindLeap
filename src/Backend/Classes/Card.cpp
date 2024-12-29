// Card.cpp
#include <QSqlQuery>
#include <QSqlError>
#include <iostream>
#include "Backend/Classes/Card.hpp"
#include "Backend/Database/setup.hpp"

// Constructor
Card::Card(QString n, QString q, QString a)
    : name(n), question(q), answer(a), id(-1) {}

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

// Setters
void Card::setID(int newID) {
    this->id = newID;
}

// Database Operations
bool Card::createCard(int deck_id) {
    const Database *db = Database::getInstance();
    QSqlQuery sqlQuery(db->getDB());

    sqlQuery.prepare("INSERT INTO Cards (question, answer, deck_id) VALUES (?, ?, ?);");
    sqlQuery.addBindValue(this->question);
    sqlQuery.addBindValue(this->answer);
    sqlQuery.addBindValue(deck_id);

    if (!sqlQuery.exec()) {
        std::cerr << "[DB] Failed to execute query: "
                  << sqlQuery.lastError().text().toStdString() << "\n";
        return false;
    }

    this->id = sqlQuery.lastInsertId().toInt();
    return true;
}

// Comparison operator
bool Card::operator==(const Card &card) const {
    return this->id == card.id;
}