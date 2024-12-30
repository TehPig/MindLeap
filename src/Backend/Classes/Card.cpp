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
Card::Card(QString q, QString a)
    : question(q), answer(a) {}

// Getters

QString Card::getQuestion() const {
    return this->question;
}

QString Card::getAnswer() const {
    return this->answer;
}

// Database Operations

bool Card::createCard() const {
    const Database *db = Database::getInstance();
    QSqlQuery sqlQuery(db->getDB());

    std::string id;
    bool idExists;

    do {
        // Generate a unique ID
        id = generateID();
        std::cout << "Generated ID: " << id << "\n";

        sqlQuery.prepare("INSERT INTO Cards (id, question, answer) "
                         "SELECT ?, ?, ? WHERE NOT EXISTS (SELECT 1 FROM Cards WHERE id = ?);");
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