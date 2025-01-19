//
// Created by TehPig on 1/7/2025.
//
#include <QString>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

#include "Backend/Classes/Stats/UserStats.hpp"
#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/generateID.hpp"

QString createUniqueUser(const QString& username, UserStats& stats) {
    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    QString userId;
    bool idExists;

    do {
        // Generate a unique ID
        userId = generateID();
        qDebug() << "[Debug - Deck] Generated ID:" << userId << "for name" << username;

        // Insert the user into the Users table
        query.prepare(QStringLiteral("INSERT INTO Users (id, username) VALUES (?, ?);"));
        query.addBindValue(userId);
        query.addBindValue(username);

        if (!query.exec()) {
            if (query.lastError().nativeErrorCode() == QStringLiteral("19")) { // SQLite constraint violation error code
                qDebug() << "[DB] ID already exists, generating a new one.";
                idExists = true;
            } else {
                qDebug() << "[DB] Failed to create user:" << query.lastError().text();
                return {};
            }
        } else idExists = false;
    } while (idExists);

    // Initialize user stats (not the right place to do it)
    stats.setUserID(userId);

    const bool stats_status = stats.initialize();
    if (!stats_status) {
        qDebug() << "[DB] Failed to initialize user stats.";
        return {};
    }

    return userId; // Return the generated user ID, successful creation
}
