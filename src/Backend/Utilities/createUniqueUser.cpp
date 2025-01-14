//
// Created by TehPig on 1/7/2025.
//
#include <QString>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

#include "Backend/Utilities/createUniqueUser.hpp"
#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/generateID.hpp"

QString createUniqueUser(const QString& username) {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());
    QString userId;
    bool idExists;

    do {
        // Generate a unique ID
        userId = QString::fromStdString(generateID());
        qDebug() << "[Debug - Deck] Generated ID: " << userId << " for name " << username;

        // Insert the user into the Users table
        query.prepare(QStringLiteral("INSERT INTO Users (id, username) VALUES (?, ?);"));
        query.addBindValue(userId);
        query.addBindValue(username);

        if (!query.exec()) {
            if (query.lastError().nativeErrorCode() == QStringLiteral("19")) { // SQLite constraint violation error code
                qDebug() << "[DB] ID already exists, generating a new one.";
                idExists = true;
            } else {
                qDebug() << "[DB] Failed to create deck: " << query.lastError().text();
                return {};
            }
        } else idExists = false;
    } while (idExists);

    qDebug() << "User created successfully with ID: " << userId;
    return userId; // Return the generated user ID, successful creation
}