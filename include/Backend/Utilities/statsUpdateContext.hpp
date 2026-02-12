#ifndef STATSUPDATECONTEXT_HPP
#define STATSUPDATECONTEXT_HPP

enum class StatsUpdateType {
    Card,
    Deck,
    User
};

struct CardUpdate {
    bool update_times_seen = false;
    bool update_start_study = false;
    bool update_last_seen = false;
    bool update_repetitions = false;
    bool update_ease_factor = false;
    bool update_interval = false;
    bool update_time_spent = false;
    int time_spent_increment = 0;
};

struct DeckUpdate {
    bool update_card_added = false;
    bool update_start_study = false;
    bool update_cards_seen = false;
    bool update_time_spent = false;
    int time_spent_increment = 0;
};

struct UserUpdate {
    bool update_cards_seen = false;
    bool update_time_spent = false;
    bool update_button_counts = false;
    int pressed_again = 0;
    int pressed_hard = 0;
    int pressed_good = 0;
    int pressed_easy = 0;
    int time_spent_increment = 0;
    int cards_seen_increment = 0;
};

struct StatsUpdateContext {
    StatsUpdateType type;
    CardUpdate card;
    DeckUpdate deck;
    UserUpdate user;

    StatsUpdateContext() : type(StatsUpdateType::Deck) {}
    StatsUpdateContext(StatsUpdateType t) : type(t) {}
};

#endif