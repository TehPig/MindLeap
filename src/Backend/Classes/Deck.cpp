#include <vector>

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

#include "Backend/Classes/Deck.hpp"
#include "Backend/Database/setup.hpp"
#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Utilities/createUniqueDeck.hpp"
#include "Backend/Classes/Algorithms/SM2.hpp"
#include "Backend/Classes/Algorithms/Leitner.hpp"

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
    logAction("Create Deck");

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Retrieve saved deck ID
    query.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Could not retrieve user ID - create Deck:" << query.lastError().text();
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
        qDebug() << "[DB] Failed to check for deck:" << query.lastError().text();
        return false;
    }

    const QString deckName = this->name.isEmpty() ? "Default" : this->name;
    if (query.next() && query.value(0).toInt() > 0) {
        // Check is required to avoid "" in debug output
        if (this->name.isEmpty()) qDebug() << "[DB] Default deck already exists. For a new deck, specify a name.";
        else qDebug() << "[DB] Deck" << deckName << "already exists.";
        return false;
    }

    if (!query.exec()) {
        qDebug() << "[DB] Failed to create deck settings:" << query.lastError().text();
        return false;
    }

    const QString res = createUniqueDeck(deckName, this->stats);
    if (res.isEmpty()) return false;

    this->id = res;
    return true;
}

// Delete Deck
bool Deck::_delete() const {
    logAction("Delete Deck");

    if (this->id.isEmpty()) {
        qDebug() << "[DB] Deck Delete - Missing ID.";
        return false;
    }

    const Database *db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare("DELETE FROM Decks WHERE id = ?;");
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to delete deck:" << query.lastError().text();
        return false;
    }

    return true; // DecksCards and DeckStats are automatically cleaned up by cascading rules.
}

// Fetch Deck from database
bool Deck::fetch() {
    logAction("Fetch Deck");

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    if (!this->id.isEmpty()) {
        query.prepare(QStringLiteral("SELECT * FROM Decks WHERE id = ?;"));
        query.addBindValue(this->id);

        if (!query.exec() || !query.next()) {
            qDebug() << "[DB] Deck Fetch - Failed to fetch deck by ID:" << query.lastError().text();
            return false;
        }

        this->name = query.value("name").toString();
    } else if (!this->name.isEmpty()) {
        query.prepare(QStringLiteral("SELECT * FROM Decks WHERE name = ?;"));
        query.addBindValue(this->name);

        if (!query.exec() || !query.next()) {
            qDebug() << "[DB] Deck Fetch - Failed to fetch deck by name:" << query.lastError().text();
            return false;
        }

        this->id = query.value("id").toString();
    } else {
        qDebug() << "[DB] Deck Fetch - Unable to fetch without ID or username.";
        return false;
    }

    return true;
}

// Add Card to Deck
bool Deck::addCard(Card& card) {
    logAction("Add Card to Deck");

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

    query.prepare(QStringLiteral("SELECT id FROM SavedUser LIMIT 1"));
    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Could not fetch saved user:" << query.lastError().text();
        return false;
    }

    const QString user_id = query.value(0).toString();

    query.prepare(QStringLiteral("INSERT INTO DecksCards (deck_id, card_id) VALUES (?, ?)"));
    query.addBindValue(this->id);
    query.addBindValue(card.getID());

    if (!query.exec()) {
        qDebug() << "[DB] Could not link Card and Deck:" << query.lastError().text();
        return false;
    }

    CardStats card_stats;
    card_stats.setCardID(card.getID());
    card_stats.setUserID(user_id);
    if (!card_stats.initialize()) {
        qDebug() << "[DB - CardStats] Failed to save stats for deck ID" << this->id;
        return false;
    }

    // Update Deck Stats
    stats.setDeckID(this->id);

    StatsUpdateContext context;
    context.type = StatsUpdateType::Deck;
    context.deck.card_added = true;
    const bool deck_status_updated = this->stats.update(context);
    if (!deck_status_updated) {
        qDebug() << "[DB - DeckStats] Failed to update deck stats.";
        return false;
    }

    qDebug() << "[DB - Deck] Successfully added card to deck with initialized stats.";
    return true;
}

// Rename Deck
bool Deck::rename(const QString& newName) {
    logAction("Rename Deck");

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
        qDebug() << "[DB] Failed to execute query:" << query.lastError().text();
        return false;
    }

    qDebug() << "[Deck] Renamed deck to" << newName;
    this->name = newName;
    return true;
}

// Set Deck description
bool Deck::setDescription(const QString& description) const {
    logAction("Set Deck Description");

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
        qDebug() << "[DB] Could not update Deck description:" << query.lastError().text();
        return false;
    }

    return true;
}

// Get Deck description
QString Deck::getDescription() const {
    logAction("Get Deck Description");

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT description FROM Decks WHERE id = ?"));
    query.addBindValue(this->id);

    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Could not retrieve Deck description:" << query.lastError().text();
        return {};
    }

    return query.value("description").toString();
}

// List all Deck cards
std::vector<Card> Deck::listCards() const {
    logAction("List Deck Cards");

    std::vector<Card> cards;
    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT id, question, answer, type FROM Cards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query:" << query.lastError().text();
        return cards;
    }

    while (query.next()) {
        //studyQueue.push(Card(query.value("id").toString(), query.value("question").toString(), query.value("answer").toString(), Card::stringToType(query.value("type").toString())));
    }

    return cards;
}

// Get card count in the deck
int Deck::getCardCount() const {
    logAction("Get Deck Card Count");

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    query.prepare(QStringLiteral("SELECT COUNT(*) FROM DecksCards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query:" << query.lastError().text();
        return -1;
    }

    return query.next() ? query.value(0).toInt() : 0;
}

std::vector<int> Deck::getCardInformation() const {
    logAction("Get Deck Card Information");

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch all cards
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM DecksCards WHERE deck_id = ?"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query:" << query.lastError().text();
        return {};
    }

    query.prepare(QStringLiteral(R"(
        SELECT SUM(CASE WHEN Cards.type = 'New' THEN 1 ELSE 0 END),
               SUM(CASE WHEN Cards.type = 'Learning' THEN 1 ELSE 0 END),
               SUM(CASE WHEN Cards.type = 'Review' THEN 1 ELSE 0 END)
        FROM Cards
        INNER JOIN DecksCards ON Cards.id = DecksCards.card_id
        WHERE DecksCards.deck_id = ?
    )"));
    query.addBindValue(this->id);

    if (!query.exec()) {
        qDebug() << "[DB] Failed to execute query:" << query.lastError().text();
        return {};
    }

    if (!query.next()) return {};

    return {
        query.value(0).toInt(),
        query.value(1).toInt(),
        query.value(2).toInt()
    };
}

// Studying Process

// Start study session
bool Deck::study() {
    logAction("Deck Study");

    if (this->id.isEmpty()) {
        qDebug() << "[DB] Deck Study - Missing ID.";
        return false;
    }

    if (getCardCount() == 0) {
        qDebug() << "[DB] Deck Study - No cards in the deck.";
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
        qDebug() << "[DB] Failed to fetch deck settings:" << query.lastError().text();
        return false;
    }

    dailyNewCardLimit = query.value(0).toInt();
    maxReviewCards = query.value(1).toInt();

    // Load DeckStats
    DeckStats deckStats;
    deckStats.setDeckID(this->id);
    if (!deckStats.load()) {
        // Initialize DeckStats if no record exists
        if (!deckStats.initialize()) {
            qDebug() << "[DB] Failed to initialize deck stats.";
            return false;
        }
    }

    const int cardsSeen = deckStats.getCardsSeen();
    // Check if the seen cards are less than the daily card limit and review limit
    if (cardsSeen >= maxReviewCards) {
        qDebug() << "[Info] Review card limit reached.";
        return false;
    }

    // Initialize session start time
    StatsUpdateContext context;
    context.type = StatsUpdateType::Deck;
    context.deck.start_study = true;
    if (!deckStats.update(context)) {
        qDebug() << "[DB] Could not update deck stats on study:" << query.lastError().text();
        return false;
    }

    qDebug() << "[Info] Studying Deck" << this->id;

    // Fetch due and learning cards for studying
    query.prepare(QStringLiteral(R"(
        SELECT c.id, c.question, c.answer, c.type,
               (cs.last_seen + cs.interval) AS due_date
        FROM Cards c
        INNER JOIN DecksCards dc ON c.id = dc.card_id
        INNER JOIN CardStats cs ON c.id = cs.id
        WHERE dc.deck_id = ?
          AND (cs.last_seen + cs.interval <= ? OR c.type = ?)
        ORDER BY due_date ASC
        LIMIT ?
    )"));
    query.addBindValue(this->id);
    query.addBindValue(QDateTime::currentSecsSinceEpoch());  // Current timestamp in seconds
    query.addBindValue(static_cast<int>(CardType::Learning));  // Card type (1 for Learning)
    query.addBindValue(dailyNewCardLimit);  // Limit on number of new cards

    if (!query.exec()) {
        qDebug() << "[DB] Could not retrieve cards for study:" << query.lastError().text();
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

    qDebug() << "[Info] Study session ready. Cards loaded:" << this->studyQueue.size();
    return true;
}

// Ends study session
bool Deck::endStudy() const {
    logAction("End Deck Study");

    if (this->id.isEmpty()) {
        qDebug() << "[DB] Deck Study - Missing ID.";
        return false;
    }

    const Database* db = Database::getInstance();
    QSqlQuery query(db->getDB());

    // Fetch session start time and time spent from DeckStats
    DeckStats deckStats;
    deckStats.setDeckID(this->id);

    const Stats* stats = deckStats.load();
    if (!stats) {
        // Initialize DeckStats if no record exists
        if (!deckStats.initialize()) {
            qDebug() << "[DB] Failed to initialize deck stats.";
            return false;
        }
    }

    const qint64 sessionStartTimeSecs = deckStats.getSessionStartTime();
    const qint64 existingTimeSpent = deckStats.getTimeSpent();
    const QDateTime sessionStartTime = QDateTime::fromSecsSinceEpoch(sessionStartTimeSecs);

    if (existingTimeSpent < 0) {
        qDebug() << "[Error] Invalid existing time spent:" << existingTimeSpent;
        return false;
    }

    const qint64 totalTimeSpent = existingTimeSpent + (QDateTime::currentDateTime().toSecsSinceEpoch() - sessionStartTimeSecs);

    // Fetch selected user
    query.prepare("SELECT id FROM SavedUser LIMIT 1");
    if (!query.exec() || !query.next()) {
        qDebug() << "[DB] Could not fetch saved user:" << query.lastError().text();
        return false;
    }

    deckStats.setUserID(query.value(0).toString());

    StatsUpdateContext context;
    context.type = StatsUpdateType::Deck;
    context.deck.time_spent = totalTimeSpent;

    if (!deckStats.update(context)) {
        qDebug() << "[Error] Failed to update deck stats.";
        return false;
    }

    qDebug() << "[Study] Study Session for deck" << this->id << "ended successfully.";
    return true;
}

// Process card response on button press
bool Deck::processCardResponse(Card& card, const int buttonPressed) {
    if (this->studyQueue.empty()) {
        qDebug() << "[Info] Study Queue is empty.";
        return false;
    }
    if (this->id.isEmpty()) {
        qDebug() << "[DB] Process Card Response - Missing ID.";
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

    // Load card stats
    CardStats cardStats;
    cardStats.setCardID(card.getID());
    cardStats.setUserID(this->id);

    if (!cardStats.load()) {
        qDebug() << "[DB] Failed to load card stats. Initializing new stats.";
        if (!cardStats.initialize()) {
            qDebug() << "[DB] Failed to initialize card stats.";
            return false;
        }
    }

    // Apply the algorithm
    SM2Algorithm sm2;
    LeitnerAlgorithm leitner;

    if (algorithm == "SM2") sm2.calculateInterval(cardStats, buttonPressed);
    else if (algorithm == "leitner") leitner.calculateInterval(cardStats, buttonPressed);
    else {
        qDebug() << "[Error] Unknown algorithm:" << algorithm;
        return false;
    }

    qDebug() << cardStats.getTimeSpent();

    // Update the card's stats
    StatsUpdateContext context;
    context.type = StatsUpdateType::Card;
    if (algorithm == "SM2") {
        context.card.repetitions = true;
        context.card.ease_factor = cardStats.getEaseFactor();
    }
    context.card.interval = cardStats.getInterval();
    context.card.last_seen = true;
    context.card.times_seen = true;
    context.card.time_spent = QDateTime::currentSecsSinceEpoch() - cardStats.getCardStartTime();

    qDebug() << cardStats.getInterval();

    if (!cardStats.update(context)) {
        qDebug() << "[DB] Failed to update card stats:" << query.lastError().text();
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
        qDebug() << "[DB] Failed to update DeckStats:" << query.lastError().text();
        return false;
    }

    return true;
}

// Get Next Card
Card Deck::getNextCard() {
    logAction("Get Deck Next Card");

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
        qDebug() << "[DB] Failed to fetch deck settings:" << query.lastError().text();
        return {};
    }

    const int dailyNewCardLimit = query.value(0).toInt();

    // Load DeckStats
    DeckStats deckStats;
    deckStats.setDeckID(this->id);
    Stats* loadedStats = deckStats.load();

    if (!loadedStats) {
        // Initialize DeckStats if no record exists
        if (!deckStats.initialize()) {
            qDebug() << "[DB] Failed to initialize deck stats.";
            return {};
        }
    }

    // Fetch cards seen count from DeckStats
    int dailyNewCardsStudied = deckStats.getCardsSeen();

    Card nextCard = this->studyQueue.front();
    this->studyQueue.pop();

    // Check if the next card is a "New" card and the daily limit is reached
    if (nextCard.getType() == CardType::New) {
        if (dailyNewCardsStudied >= dailyNewCardLimit) {
            qDebug() << "[Info] Daily new card limit reached.";
            return {};
        }
    }

    // Set the timer in the database using CardStats class
    CardStats cardStats;
    cardStats.setCardID(nextCard.getID());
    cardStats.setUserID(this->id);

    if (!cardStats.load()) {
        qDebug() << "[DB] Failed to load card stats. Initializing new stats.";
        if (!cardStats.initialize()) {
            qDebug() << "[DB] Failed to initialize card stats.";
            return {};
        }
    }

    StatsUpdateContext context;
    context.type = StatsUpdateType::Card;
    context.card.start_study = true;

    if (!cardStats.update(context)) {
        qDebug() << "[DB] Failed to set card timer.";
        return {};
    }

    return nextCard;
}

// Get Deck stats
void Deck::getStats() const {
    qDebug() << "Deck Stats";
}
