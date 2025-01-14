//
// Created by TehPig on 1/7/2025.
//
#include <QSqlError>
#include <QSqlQuery>

#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/generateID.hpp"
#include "Backend/Utilities/createUniqueDeck.hpp"

QString createUniqueDeck(const QString& name){
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());
    QString deckId;
    bool idExists;

    do {
        // Generate a unique ID
        deckId = QString::fromStdString(generateID());
        qDebug() << "[Debug - Deck] Generated ID: " << deckId << " for name " << name;

        // Insert the user into the Users table
        query.prepare(QStringLiteral("INSERT INTO Decks (id, name) VALUES (?, ?);"));
        query.addBindValue(deckId);
        query.addBindValue(name);

        if (!query.exec()) {
            if (query.lastError().nativeErrorCode() == QStringLiteral("19")) { // SQLite constraint violation error code
                qDebug() << "[DB] ID already exists, generating a new one.";
                idExists = true;
            } else {
                qDebug() << "[DB] Failed to create user: " << query.lastError().text();
                return {};
            }
        } else idExists = false;
    } while (idExists);

    qDebug() << "Deck created successfully with ID: " << deckId;
    return deckId;
}
