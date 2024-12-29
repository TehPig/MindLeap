#ifndef QUERIES_HPP
#define QUERIES_HPP

inline auto CREATE_USERS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS Users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT NOT NULL UNIQUE,
        times_seen INTEGER,
        last_viewed INTEGER,
        next_review INTEGER
    );
)";

inline auto CREATE_DECKS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS Decks (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL UNIQUE,
        user_id INTEGER NOT NULL,
        FOREIGN KEY(user_id) REFERENCES Users(id)
    );
)";

inline auto CREATE_CARDS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS Cards (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        question TEXT NOT NULL,
        answer TEXT NOT NULL,
        deck_id INTEGER NOT NULL,
        FOREIGN KEY(deck_id) REFERENCES Decks(id)
    );
)";

inline auto CREATE_DECKS_CARDS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS DecksCards (
        deck_id INTEGER NOT NULL,
        card_id INTEGER NOT NULL,
        PRIMARY KEY(deck_id, card_id),
        FOREIGN KEY(deck_id) REFERENCES Decks(id) ON DELETE CASCADE,
        FOREIGN KEY(card_id) REFERENCES Cards(id) ON DELETE CASCADE
    );
)";

inline auto CREATE_SAVED_USER_TABLE = R"(
    CREATE TABLE IF NOT EXISTS SavedUser (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL UNIQUE,
        FOREIGN KEY(user_id) REFERENCES Users(id)
    );
)";

inline auto CREATE_USER_STATS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS UserStats (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        date DATE NOT NULL,
        new_cards INTEGER DEFAULT 0,
        relearning_cards INTEGER DEFAULT 0,
        review_cards INTEGER DEFAULT 0,
        time_spent_seconds INTEGER DEFAULT 0,
        FOREIGN KEY(user_id) REFERENCES Users(id)
    );
)";

inline auto CREATE_CARD_STATS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS CardStats (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        card_id INTEGER NOT NULL,
        user_id INTEGER NOT NULL,
        date DATE NOT NULL,
        times_seen INTEGER DEFAULT 0,
        correct_answers INTEGER DEFAULT 0,
        incorrect_answers INTEGER DEFAULT 0,
        time_spent_seconds INTEGER DEFAULT 0,
        FOREIGN KEY(card_id) REFERENCES Cards(id) ON DELETE CASCADE,
        FOREIGN KEY(user_id) REFERENCES Users(id) ON DELETE CASCADE
    );
)";

inline auto CREATE_CARD_STATS_INDEXES = R"(
    CREATE INDEX IF NOT EXISTS idx_card_stats_card_id ON CardStats(card_id);
    CREATE INDEX IF NOT EXISTS idx_card_stats_user_id ON CardStats(user_id);
    CREATE INDEX IF NOT EXISTS idx_card_stats_date ON CardStats(date);
)";

#endif // QUERIES_HPP