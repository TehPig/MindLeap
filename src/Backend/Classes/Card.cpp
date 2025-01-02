// Card.cpp
#include <QSqlQuery>
#include <QSqlError>
#include <iostream>

#include <sstream>
#include <random>

#include "Backend/Utilities/generateID.hpp"
#include "Backend/Classes/Card.hpp"
#include "Backend/Database/setup.hpp"

// Constructor
Card::Card(const QString& id, const QString& q, const QString& a)
: id(id), question(q), answer(a) {}
Card::Card(const QString& q, const QString& a)
: question(q), answer(a) {}
Card::Card(const QString& id)
    : id(id) {}

// Getters
QString Card::getID() const { return this->id; }
QString Card::getQuestion() const { return this->question; }
QString Card::getAnswer() const { return this->answer; }

// Setters
void Card::setID(const QString& id) { this->id = id; }

// Database Operations

bool Card::createCard() const {
    const Database *db = Database::getInstance();
    QSqlQuery sqlQuery(db->getDB());

    std::string id;
    bool idExists;

    do {
        // Generate a unique ID
        id = generateID();
        std::cout << "Card >> Generated ID: " << id << "\n";

        sqlQuery.prepare("INSERT INTO Cards (id, question, answer) "
                         "VALUES (?, ?, ?);");
        sqlQuery.addBindValue(QString::fromStdString(id));
        sqlQuery.addBindValue(this->question);
        sqlQuery.addBindValue(this->answer);
        sqlQuery.addBindValue(QString::fromStdString(id));

        if (!sqlQuery.exec()) {
            std::cerr << "[DB] Failed to execute query: "
                      << sqlQuery.lastError().text().toStdString() << "\n";
            return false;
        }

        idExists = sqlQuery.numRowsAffected() == 0;
    } while (idExists);

    std::cout << "Card created successfully with ID: " << id << "\n";
    return true;
}