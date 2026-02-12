#ifndef DISCORDMANAGER_HPP
#define DISCORDMANAGER_HPP

#include <QString>
#include <QTimer>
#include <QDateTime>

class DiscordManager {
public:
    static void initialize();
    static void updatePresence(const QString& details, const QString& state = "", const QString& iconKey = "logo");
    static void runCallbacks();
    static void shutdown();

private:
    struct PresenceData {
        QString details;
        QString state;
        QString iconKey;
    };

    static PresenceData pendingPresence;
    static bool hasPendingUpdate;
    static QList<qint64> updateTimestamps; // Track last 5 update times
    static QTimer* updateTimer;

    static void sendPresenceNow(const QString& details, const QString& state, const QString& iconKey);
};

#endif
