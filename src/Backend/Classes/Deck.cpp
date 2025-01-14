#include <utility>
#include <vector>

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

#include "Backend/Classes/Deck.hpp"
#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/createUniqueDeck.hpp"
#include "Backend/Classes/Algorithms/SM2.hpp"
#include "Backend/Classes/Algorithms/Leitner.hpp"

// Constructors
Deck::Deck(const QString& name, const std::queue<Card>& c)
    : name(name) {}
Deck::Deck(const QString& name, const QString& id) :
    name(name), id(id) {}
Deck::Deck(const QString& name_or_id) {
    if (name_or_id.startsWith("n_")) this->name = name_or_id.trimmed().mid(2);
    else this->id = name_or_id.trimmed();
}
// For returning results
Deck::Deck() = default;

// Getters
// Get Deck name
QString Deck::getName() const {
    return this->name;
}

// Get Deck ID
QString Deck::getID() const { return this->id; }

// Setters

// Database Operations
// Create Deck
bool Deck::create() {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Retrieve saved user ID
    query.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Could not retrieve user ID - create Deck: " << query.lastError().text();
        return false;
    }

    if (this->name.isEmpty()) {
        // Check if Default user exists
        query.prepare(QStringLiteral("SELECT COUNT(*) FROM Decks WHERE name = 'Default';"));
        if (!query.exec()) {
            qDebug() << "[DB] Failed to check for default user: " << query.lastError().text();
            return false;
        }

        if (query.next() && query.value(0).toInt() > 0) {
            qDebug() << "[DB] Default user already exists. For a new user, specify a username.";
            return false;
        }

        const QString res = createUniqueDeck("Default");
        if (res.isEmpty()) return false;

        return true;
    }

    const QString res = createUniqueDeck(this->name);
    if (res.isEmpty()) return false;

    return true;
}

// Delete Deck
bool Deck::_delete() const {
    if (this->id.isEmpty()) {
        qDebug() << "[DB] Deck Delete - Missing ID.";
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("DELETE FROM Decks WHERE id = ?;");
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to delete deck: " << query.lastError().text();
        return false;
    }

    return true; // DecksCards and DeckStats are automatically cleaned up by cascading rules.
}

// Fetch Deck from database
Deck Deck::fetch() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    if (!this->id.isEmpty()) {
        query.prepare(QStringLiteral("SELECT * FROM Decks WHERE id = ?;"));
        query.addBindValue(this->id);
    } else if (!this->name.isEmpty()) {
        query.prepare(QStringLiteral("SELECT * FROM Decks WHERE name = ?;"));
        query.addBindValue(this->name);
    } else {
        qDebug() << "[DB] Deck Fetch - Unable to fetch without ID or username.";
        return {};
    }

    return {
        query.value("name").toString(), query.value("id").toString()
    };
}

// Add Card to Deck
bool Deck::addCard(Card& card) const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM Cards WHERE id = ? LIMIT 1"));
    query.addBindValue(card.getID());

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Card not found in Cards table, creating Card...";
        if (!card.create()) {
            qDebug() << "[DB] Could not create Card.";
            return false;
        }
    }

    query.prepare(QStringLiteral("SELECT user_id FROM UserCards WHERE card_id = ?"));
    query.addBindValue(card.getID());

    query.prepare(QStringLiteral("INSERT INTO DecksCards (deck_id, card_id) VALUES (?, ?)"));
    query.addBindValue(this->id);
    query.addBindValue(card.getID());

    if (!query.exec()) {
        qDebug() << "[DB] Could not link Card and Deck: " << query.lastError().text();
        return false;
    }

    DeckStats stats;
    stats.setDeckID(this->id);
    stats.setUserID(query.value("user_id").toString());
    if (!stats.initializeStats()) {
        qDebug() << "[DeckStats] Failed to save stats for deck.";
        return false;
    }

    qDebug() << "[Deck] Successfully added card to deck with initialized stats.";
    return true;
}

// Rename Deck
bool Deck::rename(const QString& newName) {
    if (newName.trimmed().isEmpty()) {
        qDebug() << "[DB] Deck Rename - Invalid name specified.";
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("UPDATE Decks SET name = ? WHERE id = ?;"));
    query.addBindValue(newName);
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
        return false;
    }

    qDebug() << "[Deck] Renamed deck to " << this->name;
    this->name = newName;
    return true;
}

// Set Deck description
bool Deck::setDescription(const QString& description) const {
    if (description.trimmed().isEmpty()) {
        qDebug() << "[DB] Deck Set Description - Missing description.";
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("UPDATE Decks SET description = ? WHERE id = ?"));
    query.addBindValue(description);
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Could not update Deck description: " << query.lastError().text();
        return false;
    }

    return true;
}

// Get Deck description
QString Deck::getDescription() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT description FROM Decks WHERE id = ?"));
    query.addBindValue(this->id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Could not retrieve Deck description: " << query.lastError().text();
        return {};
    }

    return query.value("description").toString();
}

// List all Deck cards
std::vector<Card> Deck::listCards() const {
    std::vector<Card> cards;
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT id, question, answer, type FROM Cards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
        return cards;
    }

    while (query.next()) {
        //studyQueue.push(Card(query.value("id").toString(), query.value("question").toString(), query.value("answer").toString(), Card::stringToType(query.value("type").toString())));
    }

    return cards;
}

// Get card count in the deck
int Deck::getCardCount() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT COUNT(*) FROM DecksCards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query: " << query.lastError().text();
        return -1;
    }

    return query.next() ? query.value(0).toInt() : 0;
}

// Studying Process

// Start study session
bool Deck::study() {
    if (this->id.isEmpty()) {
        qDebug() << "[DB] Deck Study - Missing ID.";
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch daily limits from DeckSettings
    query.prepare(QStringLiteral(R"(
        SELECT daily_new_card_limit, max_review_cards
        FROM DeckSettings
        WHERE id = ?
    )"));
    query.addBindValue(this->id);

    int dailyNewCardLimit = 0;
    int maxReviewCards = 0;

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Failed to fetch deck settings: " << query.lastError().text();
        return false;
    }

    dailyNewCardLimit = query.value(0).toInt();
    maxReviewCards = query.value(1).toInt();

    // Fetch cards seen and session start time from DeckStats
    query.prepare(QStringLiteral(R"(
        SELECT cards_seen, session_start_time
        FROM DeckStats
        WHERE id = ? AND date = ?
    )"));
    query.addBindValue(this->id);
    query.addBindValue(QDate::currentDate());

    int cardsSeen = 0;
    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Failed to fetch cards seen: " << query.lastError().text();
        return false;
    }

    cardsSeen = query.value(0).toInt();
    QDateTime sessionStartTime = QDateTime::fromSecsSinceEpoch(query.value(1).toLongLong());

    // Check if the seen cards are less than the daily card limit and review limit
    if (cardsSeen >= maxReviewCards) {
        qDebug() << "[Info] Review card limit reached.";
        return false;
    }

    // Initialize session start time
    sessionStartTime = QDateTime::currentDateTime();
    query.prepare(QStringLiteral(R"(
        UPDATE DeckStats
        SET session_start_time = ?
        WHERE id = ? AND date = ?
    )"));
    query.addBindValue(sessionStartTime.toSecsSinceEpoch());
    query.addBindValue(this->id);
    query.addBindValue(QDate::currentDate());
    if (!query.exec()) {
        qDebug() << "[DB] Failed to initialize session start time: " << query.lastError().text();
        return false;
    }

    qDebug() << "[Info] Studying Deck: " << this->name;

    // Fetch due and learning cards for studying
    query.prepare(QStringLiteral(R"(
        SELECT c.id, c.question, c.answer, c.type,
               (cs.last_seen + cs.time_to_reappear) AS due_date
        FROM Cards c
        INNER JOIN DecksCards dc ON c.id = dc.card_id
        INNER JOIN CardStats cs ON c.id = cs.id
        WHERE dc.id = ?
          AND (cs.last_seen + cs.time_to_reappear <= ? OR c.type = ?)
        ORDER BY due_date ASC
        LIMIT ?
    )"));
    query.addBindValue(this->id);
    query.addBindValue(QDateTime::currentSecsSinceEpoch()); // Current timestamp
    query.addBindValue(static_cast<int>(CardType::Learning)); // Include learning cards explicitly
    query.addBindValue(dailyNewCardLimit); // Limit to the number of new cards allowed

    if (!query.exec()) {
        qDebug() << "[DB] Could not retrieve cards for study: " << query.lastError().text();
        return false;
    }

    int newCardsFetched = 0;
    while (query.next()) {
        Card card(
            query.value("id").toString(),
            query.value("question").toString(),
            query.value("answer").toString(),
            Card::stringToType(query.value("type").toString())
        );

        if (card.getType() == CardType::New) {
            if (newCardsFetched >= dailyNewCardLimit) continue;
            newCardsFetched++;
        }

        this->studyQueue.push(card);
    }

    if (this->studyQueue.empty()) {
        qDebug() << "[DB] No cards available for study in this deck.";
        return false;
    }

    qDebug() << "[Info] Study session ready. Cards loaded: " << this->studyQueue.size();
    return true;
}

// Ends study session
bool Deck::endStudy() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch session start time and time spent from DeckStats
    query.prepare(QStringLiteral(R"(
        SELECT session_start_time, time_spent_seconds
        FROM DeckStats
        WHERE deck_id = ? AND date = ?
    )"));
    query.addBindValue(this->id);
    query.addBindValue(QDate::currentDate());

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Failed to fetch session start time and time spent: " << query.lastError().text();
        return false;
    }

    QDateTime sessionStartTime = QDateTime::fromSecsSinceEpoch(query.value(0).toLongLong());
    qint64 existingTimeSpent = query.value(1).toLongLong();
    const QDateTime currentTime = QDateTime::currentDateTime();

    while (sessionStartTime.date() <= currentTime.date()) {
        const QDate currentDay = sessionStartTime.date();
        auto endOfDay = QDateTime(currentDay, QTime(23, 59, 59));
        const qint64 secondsForDay = std::min(endOfDay.toSecsSinceEpoch() - sessionStartTime.toSecsSinceEpoch(),
                                        currentTime.toSecsSinceEpoch() - sessionStartTime.toSecsSinceEpoch());

        existingTimeSpent += secondsForDay;

        query.prepare(R"(
            UPDATE DeckStats
            SET time_spent_seconds = time_spent_seconds + ?
            WHERE deck_id = ? AND date = ?
        )");
        query.addBindValue(existingTimeSpent);
        query.addBindValue(this->id);
        query.addBindValue(currentDay);

        if (!query.exec()) {
            qDebug() << "[Error] Failed to finalize session stats for day:" << currentDay << query.lastError();
        }

        sessionStartTime = sessionStartTime.addSecs(secondsForDay);
    }

    qDebug() << "[Study] Session ended successfully.";
    return true;
}

// Process card response on button press
// Process card response on button press
bool Deck::processCardResponse(Card& card, const int buttonPressed) {
    // Check the deck algorithm and use the corresponding function
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral(R"(
        SELECT algorithm
        FROM DeckSettings
        WHERE id = ?
    )"));
    query.addBindValue(this->id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Failed to fetch deck algorithm: " << query.lastError().text();
        return false;
    }

    const QString algorithm = query.value(0).toString();

    // Load card stats
    CardStats cardStats = CardStats::loadStats(card.getID());

    // Apply the algorithm
    if (algorithm == "SM2") {
        SM2Algorithm::calculateInterval(card, buttonPressed);
    } else if (algorithm == "leitner") {
        LeitnerAlgorithm::calculateInterval(card, buttonPressed);
    } else {
        qDebug() << "[Error] Unknown algorithm: " << algorithm;
        return false;
    }

    // Update the card's stats
    StatsUpdateContext context;
    context.time_spent = QDateTime::currentDateTime().toSecsSinceEpoch() - cardStats.getLastSeen().toSecsSinceEpoch();
    cardStats.updateStats(context);
    cardStats.setTimeToReappear(card.getInterval());

    if (!cardStats.save()) {
        qDebug() << "[DB] Failed to update card stats: " << query.lastError().text();
        return false;
    }

    // Handle Learning cards (Again or Hard)
    if (buttonPressed == 1 || buttonPressed == 2) { // Again or Hard
        card.setType(CardType::Learning);
        this->studyQueue.push(card); // Add back to queue
    } else {
        card.setType(CardType::Review);
    }

    // Update DeckStats
    query.prepare(QStringLiteral(R"(
        UPDATE DeckStats
        SET cards_seen = cards_seen + 1
        WHERE id = ? AND date = ?
    )"));
    query.addBindValue(this->id);
    query.addBindValue(QDate::currentDate());

    if (!query.exec()) {
        qDebug() << "[DB] Failed to update DeckStats: " << query.lastError().text();
        return false;
    }

    return true;
}

// Get Next Card
Card Deck::getNextCard() {
    if (this->studyQueue.empty()) {
        qDebug() << "[Info] Study Queue is empty.";
        return {};
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch dailyNewCardLimit from DeckSettings
    query.prepare(QStringLiteral(R"(
        SELECT daily_new_card_limit
        FROM DeckSettings
        WHERE id = ?
    )"));
    query.addBindValue(this->id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Failed to fetch deck settings: " << query.lastError().text();
        return {};
    }

    const int dailyNewCardLimit = query.value(0).toInt();

    // Fetch cards seen count from DeckStats
    query.prepare(QStringLiteral(R"(
        SELECT cards_seen
        FROM DeckStats
        WHERE id = ? AND date = ?
    )"));
    query.addBindValue(this->id);
    query.addBindValue(QDate::currentDate());

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Failed to fetch cards seen count: " << query.lastError().text();
        return {};
    }

    const int dailyNewCardsStudied = query.value(0).toInt();

    Card nextCard = this->studyQueue.front();
    this->studyQueue.pop();

    // Check if the next card is a "New" card and the daily limit is reached
    if (nextCard.getType() == CardType::New) {
        if (dailyNewCardsStudied >= dailyNewCardLimit) {
            qDebug() << "[Info] Daily new card limit reached.";
            return {};
        }
    }

    // Set the timer in the database
    query.prepare(QStringLiteral(R"(
        UPDATE CardStats
        SET card_start_time = ?
        WHERE id = ?
    )"));
    query.addBindValue(QDateTime::currentDateTime().toSecsSinceEpoch());
    query.addBindValue(nextCard.getID());

    if (!query.exec()) {
        qDebug() << "[DB] Failed to set card timer: " << query.lastError().text();
        return {};
    }

    return nextCard;
}

// Get Deck stats
void Deck::getStats() const {
    qDebug() << "Deck Stats";
}