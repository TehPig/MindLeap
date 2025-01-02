#ifndef QUERIES_HPP
#define QUERIES_HPP

inline auto CREATE_USERS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS Users (
        id TEXT PRIMARY KEY,
        username TEXT NOT NULL UNIQUE
    );
)";

inline auto CREATE_DECKS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS Decks (
        id TEXT PRIMARY KEY,
        name TEXT NOT NULL,
        description TEXT
    );
)";

inline auto CREATE_CARDS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS Cards (
        id TEXT PRIMARY KEY,
        question TEXT NOT NULL,
        answer TEXT NOT NULL
    );
)";

// Junction tables

inline auto CREATE_DECKS_CARDS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS DecksCards (
        deck_id TEXT NOT NULL,
        card_id TEXT NOT NULL,
        PRIMARY KEY(deck_id, card_id),
        FOREIGN KEY(deck_id) REFERENCES Decks(id) ON DELETE CASCADE,
        FOREIGN KEY(card_id) REFERENCES Cards(id) ON DELETE CASCADE
    );
)";

inline auto CREATE_USERS_DECKS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS UsersDecks (
        user_id TEXT NOT NULL,
        deck_id TEXT NOT NULL,
        PRIMARY KEY(user_id, deck_id),
        FOREIGN KEY(user_id) REFERENCES Users(id) ON DELETE CASCADE,
        FOREIGN KEY(deck_id) REFERENCES Decks(id) ON DELETE CASCADE
    );
)";

// Utility tables

inline auto CREATE_SAVED_USER_TABLE = R"(
    CREATE TABLE IF NOT EXISTS SavedUser (
        id TEXT NOT NULL UNIQUE,
        FOREIGN KEY(id) REFERENCES Users(id)
    );
)";

// Stats tables

// Stats include
// Time spent in seconds
// Number of cards viewed
// List hard and easy cards
// Times used the app within the day
// Stats for each button (again, hard, good, easy)

inline auto CREATE_USER_STATS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS UserStats (
        id TEXT PRIMARY KEY,
        date DATE NOT NULL,
        cards_seen INTEGER DEFAULT 0,
        pressed_again INTEGER DEFAULT 0,
        pressed_hard INTEGER DEFAULT 0,
        pressed_good INTEGER DEFAULT 0,
        pressed_easy INTEGER DEFAULT 0,
        time_spent_seconds INTEGER DEFAULT 0,
        times_used INTEGER DEFAULT 0,
        FOREIGN KEY(id) REFERENCES Users(id)
    );
)";

// Card Stats include
// Number of times seen
// Date and times seen will be used to calculate times appeared in a day
// Time to reappear
// - first button (again) will have a multiplier of 0.0 if pressed (resets interval)
// - second button (hard) will have a multiplier of 1.2 if pressed (increases interval by 20%)
// - third button (good) will have a multiplier of 1.5 if pressed (increases interval by 50%)
// - fourth button (easy) will have a multiplier of 2.0 if pressed (doubles the interval)
// - harder cards reappear sooner e.g., 1 minute, 10 minutes, 1 day, 3 days
// - easier cards reappear later e.g., 1 day, 3 days, 1 week, 1 month
// Last seen timestamp (can be used to calculate time to reappear)

inline auto CREATE_CARD_STATS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS CardStats (
        id TEXT PRIMARY KEY,
        user_id TEXT NOT NULL,
        date DATE NOT NULL,
        times_seen INTEGER DEFAULT 0,
        time_spent_seconds INTEGER DEFAULT 0,
        time_to_reappear INTEGER DEFAULT 0,
        last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(id) REFERENCES Cards(id) ON DELETE CASCADE,
        FOREIGN KEY(user_id) REFERENCES Users(id) ON DELETE CASCADE
    );
)";

inline auto CARD_STATS_CARD_INDEX = R"(
    CREATE INDEX IF NOT EXISTS idx_card_stats_card_id ON CardStats(id);
)";
inline auto CARD_STATS_USER_INDEX = R"(
    CREATE INDEX IF NOT EXISTS idx_card_stats_user_id ON CardStats(user_id);
)";
inline auto CARD_STATS_DATE_INDEX = R"(
    CREATE INDEX IF NOT EXISTS idx_card_stats_date ON CardStats(date);
)";

#endif // QUERIES_HPP
