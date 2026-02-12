#include <vector>

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

#include "Backend/Utilities/Logger.hpp"
#include "Backend/Classes/Deck.hpp"
#include "Backend/Classes/Stats/UserStats.hpp"
#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Utilities/createUniqueDeck.hpp"
#include "Backend/Classes/Algorithms/SM2.hpp"
#include "Backend/Classes/Algorithms/Leitner.hpp"

// DEBUG: Speed up time by changing the interval multiplier (default 86400 for days)
// Set this to 60 to treat intervals as minutes for testing.
const int STUDY_INTERVAL_MULTIPLIER = 86400; 
// Constructors
Deck::Deck(const QString& name, const std::queue<Card>& c)
    : name(name), stats() {}
Deck::Deck(const QString& name, const QString& id) :
    name(name), id(id), stats() {}
Deck::Deck(const QString& name_or_id) {
    if (name_or_id.startsWith("n_")) this->name = name_or_id.trimmed().mid(2);
    else this->id = name_or_id.trimmed();
}
// For returning results
Deck::Deck() = default;

// Getters
// Get Deck name
QString Deck::getName() const { return this->name; }

// Get Deck ID
QString Deck::getID() const { return this->id; }

// Get Study Queue Size
int Deck::getStudyQueueSize() const { return this->studyQueue.size(); }

// Setters

// Database Operations
// Create Deck
bool Deck::create() {
    Logger::entity("Creating Deck", this->name);

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Retrieve saved deck ID
    query.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!query.exec() || !query.next()) {
        Logger::error("Could not retrieve user ID for deck creation", "Deck");
        return false;
    }

    const QString user_id = query.value(0).toString();

    // Check if deck with the given name exists for that user
    query.prepare(QStringLiteral(R"(
            SELECT COUNT(*)
            FROM Decks d
            INNER JOIN UsersDecks ud ON d.id = ud.deck_id
            WHERE ud.user_id = ? AND d.name = ?;
        )"));
    query.addBindValue(user_id);
    query.addBindValue(this->name.isEmpty() ? "Default" : this->name);
    if (!query.exec()) {
        Logger::error("Failed to check for existing deck", "Deck");
        return false;
    }

    const QString deckName = this->name.isEmpty() ? "Default" : this->name;
    if (query.next() && query.value(0).toInt() > 0) {
        Logger::warn(QString("Deck '%1' already exists").arg(deckName), "Deck");
        return false;
    }

    const QString res = createUniqueDeck(deckName, this->stats);
    if (res.isEmpty()) return false;

    this->id = res;
    return true;
}

// Delete Deck
bool Deck::_delete() const {
    Logger::entity("Deleting Deck", this->id);

    if (this->id.isEmpty()) {
        Logger::error("Deck Delete - Missing ID", "Deck");
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("DELETE FROM Decks WHERE id = ?;");
    query.addBindValue(this->id);

    if (!query.exec()) {
        Logger::error("Failed to delete deck: " + query.lastError().text(), "Deck");
        return false;
    }

    return true;
}

// Fetch Deck from database
bool Deck::fetch() {
    Logger::entity("Fetching Deck", this->id);

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    if (!this->id.isEmpty()) {
        query.prepare(QStringLiteral("SELECT * FROM Decks WHERE id = ?;"));
        query.addBindValue(this->id);

        if (!query.exec() || !query.next()) {
            Logger::error("Failed to fetch deck by ID: " + query.lastError().text(), "Deck");
            return false;
        }

        this->name = query.value("name").toString();
    } else if (!this->name.isEmpty()) {
        query.prepare(QStringLiteral("SELECT * FROM Decks WHERE name = ?;"));
        query.addBindValue(this->name);

        if (!query.exec() || !query.next()) {
            Logger::error("Failed to fetch deck by name: " + query.lastError().text(), "Deck");
            return false;
        }

        this->id = query.value("id").toString();
    } else {
        Logger::error("Unable to fetch deck without ID or name", "Deck");
        return false;
    }

    return true;
}

// Add Card to Deck
bool Deck::addCard(Card& card) {
    Logger::entity("Adding Card to Deck", card.getID());

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT * FROM Cards WHERE id = ? LIMIT 1"));
    query.addBindValue(card.getID());

    if (!query.exec() || !query.next()) {
        Logger::db("Card not found, creating Card...", "Deck");
        if (!card.create()) {
            Logger::error("Could not create Card", "Deck");
            return false;
        }
    }

    query.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!query.exec() || !query.next()) {
        Logger::error("Could not fetch saved user", "Deck");
        return false;
    }

    const QString user_id = query.value(0).toString();

    query.prepare(QStringLiteral("INSERT INTO DecksCards (deck_id, card_id) VALUES (?, ?)"));
    query.addBindValue(this->id);
    query.addBindValue(card.getID());

    if (!query.exec()) {
        Logger::error("Could not link Card and Deck", "Deck");
        return false;
    }

    // Update Deck Stats
    stats.setDeckID(this->id);
    stats.initialize(); // Ensure record for today exists

    StatsUpdateContext context;
    context.type = StatsUpdateType::Deck;
    context.deck.update_card_added = true;
    const bool deck_status_updated = this->stats.update(context);
    if (!deck_status_updated) {
        Logger::error("Failed to update deck stats", "Deck");
        return false;
    }

    Logger::info("Successfully added card to deck", "Deck");
    return true;
}

// Rename Deck
bool Deck::rename(const QString& newName) {
    Logger::entity("Renaming Deck", this->id);

    if (newName.trimmed().isEmpty()) {
        Logger::warn("Deck Rename - Invalid name specified", "Deck");
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("UPDATE Decks SET name = ? WHERE id = ?;"));
    query.addBindValue(newName);
    query.addBindValue(this->id);

    if (!query.exec()) {
        Logger::error("Failed to rename deck: " + query.lastError().text(), "Deck");
        return false;
    }

    Logger::info(QString("Renamed deck to '%1'").arg(newName), "Deck");
    this->name = newName;
    return true;
}

// Set Deck description
bool Deck::setDescription(const QString& description) const {
    Logger::entity("Setting Deck Description", this->id);

    if (description.trimmed().isEmpty()) {
        Logger::warn("Deck Set Description - Empty description", "Deck");
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("UPDATE Decks SET description = ? WHERE id = ?"));
    query.addBindValue(description);
    query.addBindValue(this->id);

    if (!query.exec()) {
        Logger::error("Could not update Deck description: " + query.lastError().text(), "Deck");
        return false;
    }

    return true;
}

// Get Deck description
QString Deck::getDescription() const {
    Logger::entity("Getting Deck Description", this->id);

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT description FROM Decks WHERE id = ?"));
    query.addBindValue(this->id);

    if (!query.exec() || !query.next()) {
        Logger::error("Could not retrieve Deck description", "Deck");
        return {};
    }

    return query.value("description").toString();
}

// List all Deck cards
std::vector<Card> Deck::listCards() const {
    Logger::entity("Listing Deck Cards", this->id);

    std::vector<Card> cards;
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT id, question, answer, type FROM Cards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        Logger::error("Failed to list cards: " + query.lastError().text(), "Deck");
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
        Logger::error("Failed to get card count: " + query.lastError().text(), "Deck");
        return -1;
    }

    return query.next() ? query.value(0).toInt() : 0;
}

std::vector<int> Deck::getCardInformation() const {
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch user ID
    QSqlQuery userQuery(db->getDB());
    userQuery.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!userQuery.exec() || !userQuery.next()) return {0, 0, 0};
    const QString currentUserID = userQuery.value(0).toString();

    // Fetch limits
    query.prepare("SELECT daily_new_card_limit, max_review_cards FROM DeckSettings WHERE id = ?");
    query.addBindValue(this->id);
    int newLimit = 20;
    int reviewLimit = 100;
    if (query.exec() && query.next()) {
        newLimit = query.value(0).toInt();
        reviewLimit = query.value(1).toInt();
    }

    // Count new cards studied today
    query.prepare(QStringLiteral(R"(
        SELECT COUNT(DISTINCT id) FROM CardStats
        WHERE user_id = ? AND date = DATE('now')
          AND id IN (SELECT card_id FROM DecksCards WHERE deck_id = ?)
          AND id NOT IN (SELECT id FROM CardStats WHERE date < DATE('now'))
    )"));
    query.addBindValue(currentUserID);
    query.addBindValue(this->id);
    int newStudiedToday = 0;
    if (query.exec() && query.next()) newStudiedToday = query.value(0).toInt();
    
    int remainingNewLimit = std::max(0, newLimit - newStudiedToday);

    // Count reviews studied today
    query.prepare(QStringLiteral(R"(
        SELECT COUNT(DISTINCT id) FROM CardStats
        WHERE user_id = ? AND date = DATE('now')
          AND id IN (SELECT card_id FROM DecksCards WHERE deck_id = ?)
          AND id IN (SELECT id FROM CardStats WHERE date < DATE('now'))
    )"));
    query.addBindValue(currentUserID);
    query.addBindValue(this->id);
    int reviewsStudiedToday = 0;
    if (query.exec() && query.next()) reviewsStudiedToday = query.value(0).toInt();

    int remainingReviewLimit = std::max(0, reviewLimit - reviewsStudiedToday);

    // Count available New cards (up to remaining limit)
    query.prepare(R"(
        SELECT COUNT(*) FROM Cards c
        INNER JOIN DecksCards dc ON c.id = dc.card_id
        WHERE dc.deck_id = ? AND c.type = 'New'
          AND c.id NOT IN (SELECT id FROM CardStats WHERE user_id = ?)
    )");
    query.addBindValue(this->id);
    query.addBindValue(currentUserID);
    int availableNew = 0;
    if (query.exec() && query.next()) availableNew = std::min(remainingNewLimit, query.value(0).toInt());

    // Count available Learning cards (No limit)
    query.prepare(R"(
        SELECT COUNT(*) FROM Cards c
        INNER JOIN DecksCards dc ON c.id = dc.card_id
        WHERE dc.deck_id = ? AND c.type = 'Learning'
    )");
    query.addBindValue(this->id);
    int availableLearn = 0;
    if (query.exec() && query.next()) availableLearn = query.value(0).toInt();

    // Count due Review cards (up to remaining limit)
    query.prepare(R"(
        SELECT COUNT(*) FROM Cards c
        INNER JOIN DecksCards dc ON c.id = dc.card_id
        INNER JOIN CardStats cs ON c.id = cs.id
        WHERE dc.deck_id = ? AND c.type = 'Review' AND cs.user_id = ?
          AND (cs.last_seen + (cs.interval * ?)) <= ?
    )");
    query.addBindValue(this->id);
    query.addBindValue(currentUserID);
    query.addBindValue(STUDY_INTERVAL_MULTIPLIER);
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    int availableReview = 0;
    if (query.exec() && query.next()) availableReview = std::min(remainingReviewLimit, query.value(0).toInt());

    return { availableNew, availableLearn, availableReview };
}

// Studying Process

// Start study session
bool Deck::study() {
    Logger::entity("Starting Study Session", this->id);

    if (this->id.isEmpty()) {
        Logger::error("Deck Study - Missing ID", "Deck");
        return false;
    }

    if (getCardCount() == 0) {
        Logger::info("Deck Study - No cards in the deck", "Deck");
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
        Logger::error("Failed to fetch deck settings: " + query.lastError().text(), "Deck");
        return false;
    }

    dailyNewCardLimit = query.value(0).toInt();
    maxReviewCards = query.value(1).toInt();

    // Load DeckStats
    DeckStats deckStats;
    deckStats.setDeckID(this->id);
    delete deckStats.load();
    if (!deckStats.initialize()) {
        Logger::error("Failed to initialize deck stats", "Deck");
        return false;
    }

    // Get current progress
    const std::vector<int> currentInfo = getCardInformation();
    const int availableNew = currentInfo[0];
    const int availableLearn = currentInfo[1];
    const int availableReview = currentInfo[2];

    if (availableNew == 0 && availableLearn == 0 && availableReview == 0) {
        Logger::info("No cards available for study (limits reached or nothing due)", "Deck");
        return false;
    }

    // Initialize session start time
    StatsUpdateContext context;
    context.type = StatsUpdateType::Deck;
    context.deck.update_start_study = true;
    if (!deckStats.update(context)) {
        Logger::error("Could not update deck stats session start", "Deck");
        return false;
    }

    // Fetch total cards in deck for logging
    QSqlQuery countQuery(db->getDB());
    countQuery.prepare("SELECT COUNT(*) FROM DecksCards WHERE deck_id = ?");
    countQuery.addBindValue(this->id);
    if (countQuery.exec() && countQuery.next()) {
        Logger::info(QString("Total cards in deck: %1").arg(countQuery.value(0).toInt()), "Deck");
    }

    // Fetch the current user's ID to bind to the query
    QSqlQuery userQuery(db->getDB());
    userQuery.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!userQuery.exec() || !userQuery.next()) {
        Logger::error("Could not retrieve user ID for study query", "Deck");
        return false;
    }
    const QString currentUserID = userQuery.value(0).toString();

    // Fetch due and learning cards for studying
    query.prepare(QStringLiteral(R"(
        SELECT c.id, c.question, c.answer, c.type,
               COALESCE(latest_cs.last_seen + (latest_cs.interval * ?), 0) AS due_date
        FROM Cards c
        INNER JOIN DecksCards dc ON c.id = dc.card_id
        LEFT JOIN (
            SELECT id, last_seen, interval, 
                   ROW_NUMBER() OVER (PARTITION BY id ORDER BY date DESC) as rn
            FROM CardStats
            WHERE user_id = ?
        ) latest_cs ON c.id = latest_cs.id AND latest_cs.rn = 1
        WHERE dc.deck_id = ?
          AND (latest_cs.id IS NULL OR (latest_cs.last_seen + (latest_cs.interval * ?)) <= ? OR c.type = 'Learning')
        ORDER BY due_date ASC
        LIMIT 1000
    )"));

    query.addBindValue(STUDY_INTERVAL_MULTIPLIER);
    query.addBindValue(currentUserID);
    query.addBindValue(this->id);
    query.addBindValue(STUDY_INTERVAL_MULTIPLIER);
    query.addBindValue(QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
        Logger::error("Could not retrieve cards for study: " + query.lastError().text(), "Deck");
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
        Logger::info("No cards available for study in this deck", "Deck");
        return false;
    }

    Logger::info(QString("Study session ready. Cards loaded: %1").arg(QString::number(this->studyQueue.size())), "Deck");
    return true;
}

// Ends study session
bool Deck::endStudy() const {
    Logger::entity("Ending Study Session", this->id);

    if (this->id.isEmpty()) {
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch session start time and time spent from DeckStats
    DeckStats deckStats;
    deckStats.setDeckID(this->id);

    delete deckStats.load();
    if (!deckStats.initialize()) {
        return false;
    }

    const qint64 sessionStartTimeSecs = deckStats.getSessionStartTime();
    const qint64 existingTimeSpent = deckStats.getTimeSpent();

    if (existingTimeSpent < 0) {
        Logger::error("Invalid existing time spent", "Deck");
        return false;
    }

    const qint64 timeSpentInSession = QDateTime::currentDateTime().toSecsSinceEpoch() - sessionStartTimeSecs;

    // Fetch selected user
    query.prepare("SELECT id FROM SavedUser LIMIT 1");
    if (!query.exec() || !query.next()) {
        Logger::error("Could not fetch saved user", "Deck");
        return false;
    }

    deckStats.setUserID(query.value(0).toString());

    StatsUpdateContext context;
    context.type = StatsUpdateType::Deck;
    context.deck.update_time_spent = true;
    context.deck.time_spent_increment = timeSpentInSession;

    if (!deckStats.update(context)) {
        Logger::error("Failed to update deck stats at session end", "Deck");
        return false;
    }

    Logger::info(QString("Study session for deck %1 ended. Session duration: %2s").arg(this->id, QString::number(timeSpentInSession)), "Deck");
    return true;
}

// Process card response on button press
bool Deck::processCardResponse(Card& card, const int buttonPressed) {
    if (this->id.isEmpty()) {
        qDebug() << "[DB] Process Card Response - Missing Deck ID.";
        return false;
    }

    if (card.getID().isEmpty()) {
        Logger::error("Process Card Response - Missing Card ID", "Deck");
        return false;
    }

    logAction("Process Deck Card Response");
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
        qDebug() << "[DB] Failed to fetch deck algorithm:" << query.lastError().text();
        return false;
    }

    const QString algorithm = query.value(0).toString();
    Logger::info("Using algorithm: " + algorithm, "Deck");

    // Fetch the current user's ID
    QSqlQuery userQuery(db->getDB());
    userQuery.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!userQuery.exec() || !userQuery.next()) {
        Logger::error("Could not retrieve user ID for CardStats", "Deck");
        return false;
    }
    const QString currentUserID = userQuery.value(0).toString();

    // Load latest card stats
    CardStats cardStats;
    cardStats.setCardID(card.getID());
    cardStats.setUserID(currentUserID);

    delete cardStats.load(); // Load latest history if available
    
    // Ensure today's record exists (carries over latest stats if needed)
    if (!cardStats.initialize()) {
        Logger::error("Failed to initialize card stats for today", "Deck");
        return false;
    }

    // Apply the algorithm
    SM2Algorithm sm2;
    LeitnerAlgorithm leitner;

    if (algorithm.toUpper() == "SM2") sm2.calculateInterval(cardStats, buttonPressed);
    else if (algorithm.toLower() == "leitner") leitner.calculateInterval(cardStats, buttonPressed);
    else {
        Logger::error("Unknown algorithm: " + algorithm, "Deck");
        return false;
    }

    // Update the card's stats
    StatsUpdateContext context;
    context.type = StatsUpdateType::Card;
    if (algorithm == "SM2") {
        context.card.update_repetitions = true;
        context.card.update_ease_factor = true;
    }
    context.card.update_interval = true;
    context.card.update_last_seen = true;
    context.card.update_times_seen = true;
    context.card.update_time_spent = true;
    context.card.time_spent_increment = QDateTime::currentSecsSinceEpoch() - cardStats.getCardStartTime();

    Logger::info(QString("Processing response for card %1 (Button: %2, Interval: %3)").arg(card.getID(), QString::number(buttonPressed), QString::number(cardStats.getInterval())), "Deck");

    if (!cardStats.update(context)) {
        Logger::error("Failed to update card stats", "Deck");
        return false;
    }

    // Update User and Deck Statistics
    UserStats userStats(currentUserID);
    userStats.initialize(); // Ensure record for today exists

    DeckStats deckStats;
    deckStats.setDeckID(this->id);
    deckStats.initialize(); // Ensure record for today exists

    StatsUpdateContext userContext(StatsUpdateType::User);
    userContext.user.update_button_counts = true;
    userContext.user.update_cards_seen = true;
    userContext.user.cards_seen_increment = 1;

    switch (buttonPressed) {
        case 1: userContext.user.pressed_again = 1; break;
        case 2: userContext.user.pressed_hard = 1; break;
        case 3: userContext.user.pressed_good = 1; break;
        case 4: userContext.user.pressed_easy = 1; break;
    }

    userStats.update(userContext);

    StatsUpdateContext deckContext(StatsUpdateType::Deck);
    deckContext.deck.update_cards_seen = true;
    deckStats.update(deckContext);

    // Handle Learning cards (Again or Hard)
    if (buttonPressed == 1 || buttonPressed == 2) { // Again or Hard
        card.setType(CardType::Learning);
        this->studyQueue.push(card); // Add back to queue
    } else {
        card.setType(CardType::Review);
    }
    
    card.saveType(); // Persist the type change to DB

    return true;
}

// Get Next Card
Card Deck::getNextCard() {
    if (this->studyQueue.empty()) {
        Logger::info("Study Queue is empty", "Deck");
        return {};
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch dailyNewCardLimit and maxReviewCards from DeckSettings
    query.prepare(QStringLiteral(R"(
        SELECT daily_new_card_limit, max_review_cards
        FROM DeckSettings
        WHERE id = ?
    )"));
    query.addBindValue(this->id);

    if (!query.exec() || !query.next()) {
        Logger::error("Failed to fetch deck settings: " + query.lastError().text(), "Deck");
        return {};
    }

    const int dailyNewCardLimit = query.value(0).toInt();
    const int maxReviewCards = query.value(1).toInt();

    // Load DeckStats
    DeckStats deckStats;
    deckStats.setDeckID(this->id);
    if (Stats* loadedStats = deckStats.load()) {
        delete loadedStats;
    } else {
        if (!deckStats.initialize()) {
            Logger::error("Failed to initialize deck stats", "Deck");
            return {};
        }
    }

    // Fetch progress from CardStats
    QSqlQuery progressQuery(db->getDB());
    
    // New cards studied today
    progressQuery.prepare(QStringLiteral(R"(
        SELECT COUNT(DISTINCT id)
        FROM CardStats
        WHERE user_id = (SELECT id FROM SavedUser LIMIT 1)
          AND date = DATE('now')
          AND id IN (SELECT card_id FROM DecksCards WHERE deck_id = ?)
          AND id NOT IN (SELECT id FROM CardStats WHERE date < DATE('now'))
    )"));
    progressQuery.addBindValue(this->id);
    int dailyNewCardsStudiedToday = 0;
    if (progressQuery.exec() && progressQuery.next()) {
        dailyNewCardsStudiedToday = progressQuery.value(0).toInt();
    }

    // Review cards studied today
    progressQuery.prepare(QStringLiteral(R"(
        SELECT COUNT(DISTINCT id)
        FROM CardStats
        WHERE user_id = (SELECT id FROM SavedUser LIMIT 1)
          AND date = DATE('now')
          AND id IN (SELECT card_id FROM DecksCards WHERE deck_id = ?)
          AND id IN (SELECT id FROM CardStats WHERE date < DATE('now'))
    )"));
    progressQuery.addBindValue(this->id);
    int dailyReviewsStudiedToday = 0;
    if (progressQuery.exec() && progressQuery.next()) {
        dailyReviewsStudiedToday = progressQuery.value(0).toInt();
    }

    Card nextCard = this->studyQueue.front();
    
    // Check limits based on card type
    if (nextCard.getType() == CardType::New) {
        if (dailyNewCardsStudiedToday >= dailyNewCardLimit) {
            Logger::info("Daily new card limit reached during session", "Deck");
            // Optionally clear the queue of other new cards? 
            // For now just stop.
            return {};
        }
    } else if (nextCard.getType() == CardType::Review) {
        if (dailyReviewsStudiedToday >= maxReviewCards) {
            Logger::info("Daily review card limit reached during session", "Deck");
            return {};
        }
    }

    this->studyQueue.pop();

    // Check if the next card is "Brand New" (has NO stats records at all)
    // This is a secondary check to ensure consistency if getType() is not 'New' but it has no stats.
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM CardStats WHERE id = ?"));
    query.addBindValue(nextCard.getID());
    
    bool isBrandNew = true;
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        isBrandNew = false;
    }

    if (isBrandNew && nextCard.getType() != CardType::New) {
         // Should not happen often but handle just in case
         if (dailyNewCardsStudiedToday >= dailyNewCardLimit) return {};
    }

    // Set the timer in the database using CardStats class
    QSqlQuery userQuery(db->getDB());
    userQuery.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!userQuery.exec() || !userQuery.next()) {
        Logger::error("Could not retrieve user ID for getNextCard", "Deck");
        return {};
    }
    const QString currentUserID = userQuery.value(0).toString();

    CardStats cardStats;
    cardStats.setCardID(nextCard.getID());
    cardStats.setUserID(currentUserID);

    if (Stats* loadedCardStats = cardStats.load()) {
        delete loadedCardStats;
    } else {
        if (!cardStats.initialize()) {
            Logger::error("Failed to initialize card stats", "Deck");
            return {};
        }
    }

    StatsUpdateContext context;
    context.type = StatsUpdateType::Card;
    context.card.update_start_study = true;

    if (!cardStats.update(context)) {
        Logger::error("Failed to set card timer", "Deck");
        return {};
    }

    return nextCard;
}

// Get Deck stats object (For UI)
DeckStats Deck::getDeckStats() const {
    DeckStats deckStats;
    deckStats.setDeckID(this->id);
    Stats* ptr = deckStats.load();
    if (ptr) delete ptr;
    return deckStats;
}

DeckStats Deck::getTotalDeckStats() const {
    DeckStats deckStats;
    deckStats.setDeckID(this->id);
    Stats* ptr = deckStats.loadTotal();
    if (ptr) delete ptr;
    return deckStats;
}

// Get Deck stats
void Deck::getStats() const {
    Logger::info("Deck Stats triggered (placeholder)", "Deck");
}
