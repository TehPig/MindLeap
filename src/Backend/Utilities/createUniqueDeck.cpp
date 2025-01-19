//
// Created by TehPig on 1/7/2025.
//
#include <QSqlError>
#include <QSqlQuery>

#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/generateID.hpp"
#include "Backend/Classes/Stats/DeckStats.hpp"

QString createUniqueDeck(const QString& name, DeckStats& stats) {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    QString deckId;
    bool idExists;

    do {
        // Generate a unique ID
        deckId = generateID();
        qDebug() << "[Debug - Deck] Generated ID:" << deckId << "for name" << name;

        // Insert the user into the Users table
        query.prepare(QStringLiteral("INSERT INTO Decks (id, name) VALUES (?, ?);"));
        query.addBindValue(deckId);
        query.addBindValue(name);

        if (!query.exec()) {
            if (query.lastError().nativeErrorCode() == QStringLiteral("19")) { // SQLite constraint violation error code
                qDebug() << "[DB] ID already exists, generating a new one.";
                idExists = true;
            } else {
                qDebug() << "[DB] Failed to create deck:" << query.lastError().text();
                return {};
            }
        } else idExists = false;
    } while (idExists);

    // Link the user to the default deck
    query.prepare(QStringLiteral("INSERT INTO UsersDecks (user_id, deck_id) VALUES ((SELECT id FROM SavedUser), ?);"));
    query.addBindValue(deckId);

    if (!query.exec()) {
        qDebug() << "[DB] Could not link user to deck:" << query.lastError().text();
        return {};
    }

    // Create deck settings
    query.prepare(QStringLiteral("INSERT INTO DeckSettings (id) VALUES (?)"));
    query.addBindValue(deckId);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to create deck settings:" << query.lastError().text();
        return {};
    }

    return deckId; // Return the generated deck ID, successful creation
}
