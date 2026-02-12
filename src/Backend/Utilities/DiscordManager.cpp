#include "Backend/Utilities/DiscordManager.hpp"
#include "Backend/RPC/discord_rpc.h"
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <cstring>

// Initialize static members
DiscordManager::PresenceData DiscordManager::pendingPresence = {"", "", ""};
bool DiscordManager::hasPendingUpdate = false;
QList<qint64> DiscordManager::updateTimestamps;
QTimer* DiscordManager::updateTimer = nullptr;
static QTimer* callbackTimer = nullptr;

void DiscordManager::initialize() {
    qDebug() << "[Discord] Initializing RPC...";
#ifdef USE_DISCORD_RPC
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    
    // Define callbacks to handle errors/connection status
    handlers.ready = [](const DiscordUser* request) {
        qDebug() << "[Discord] Connected to user:" << request->username;
    };
    handlers.disconnected = [](int errorCode, const char* message) {
        qDebug() << "[Discord] Disconnected:" << errorCode << message;
    };
    handlers.errored = [](int errorCode, const char* message) {
        qDebug() << "[Discord] Error:" << errorCode << message;
    };

    Discord_Initialize("1470970991076311239", &handlers, 1, NULL);

    // Set up a timer to run callbacks every 5 seconds
    if (!callbackTimer) {
        callbackTimer = new QTimer();
        QObject::connect(callbackTimer, &QTimer::timeout, []() {
            DiscordManager::runCallbacks();
        });
        callbackTimer->start(5000); // 5 seconds
    }

    // Set up a timer for rate-limited updates (check every 2 seconds)
    if (!updateTimer) {
        updateTimer = new QTimer();
        QObject::connect(updateTimer, &QTimer::timeout, []() {
            if (hasPendingUpdate) {
                // Try to send the pending update
                updatePresence(pendingPresence.details, pendingPresence.state, pendingPresence.iconKey);
            }
        });
        updateTimer->start(2000); 
    }
#else
    qDebug() << "[Discord] RPC disabled in this build (incompatible compiler).";
#endif
}

void DiscordManager::updatePresence(const QString& details, const QString& state, const QString& iconKey) {
#ifdef USE_DISCORD_RPC
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    
    // Prune timestamps older than 20 seconds
    while (!updateTimestamps.isEmpty() && (currentTime - updateTimestamps.first() >= 20)) {
        updateTimestamps.removeFirst();
    }

    // If we have "tokens" left (less than 5 updates in last 20s), send now
    if (updateTimestamps.size() < 5) {
        sendPresenceNow(details, state, iconKey);
        hasPendingUpdate = false;
    } else {
        // Otherwise, queue it/update the queue
        pendingPresence = {details, state, iconKey};
        hasPendingUpdate = true;
        qDebug() << "[Discord] Rate limit reached. Update queued.";
    }
#endif
}

void DiscordManager::sendPresenceNow(const QString& details, const QString& state, const QString& iconKey) {
    qDebug() << "[Discord] Sending presence update:" << details << state;
#ifdef USE_DISCORD_RPC
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));

    QByteArray stateBytes = state.toUtf8();
    QByteArray detailsBytes = details.toUtf8();
    QByteArray iconKeyBytes = iconKey.toUtf8();
    QByteArray smallImageTextBytes = QString("v%1 - By %2")
                                        .arg(PROJECT_VERSION)
                                        .arg(PROJECT_CREATOR).toUtf8();

    discordPresence.state = stateBytes.constData();
    discordPresence.details = detailsBytes.constData();
    discordPresence.startTimestamp = QDateTime::currentSecsSinceEpoch();
    discordPresence.largeImageKey = "logo";
    discordPresence.smallImageKey = iconKeyBytes.constData();
    discordPresence.smallImageText = smallImageTextBytes.constData();
    
    Discord_UpdatePresence(&discordPresence);
    updateTimestamps.append(QDateTime::currentSecsSinceEpoch());
#endif
}

void DiscordManager::runCallbacks() {
#ifdef USE_DISCORD_RPC
    Discord_RunCallbacks();
#endif
}

void DiscordManager::shutdown() {
    qDebug() << "[Discord] Shutting down RPC...";
#ifdef USE_DISCORD_RPC
    if (updateTimer) {
        updateTimer->stop();
        delete updateTimer;
        updateTimer = nullptr;
    }
    if (callbackTimer) {
        callbackTimer->stop();
        delete callbackTimer;
        callbackTimer = nullptr;
    }
    Discord_Shutdown();
#endif
}
