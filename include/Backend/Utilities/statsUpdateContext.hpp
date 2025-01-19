#ifndef STATSUPDATECONTEXT_HPP
#define STATSUPDATECONTEXT_HPP

enum class StatsUpdateType {
    Card,
    Deck,
    User
};

struct StatsUpdateContext {
    StatsUpdateType type;

    union {
        struct {
            bool times_seen;
            bool start_study;
            bool last_seen;
            bool repetitions;
            int ease_factor;
            int interval;
            int time_spent;
        } card;

        struct {
            bool card_added;
            bool start_study;
            int cards_seen;
            int time_spent;
        } deck;

        struct {
            int pressed_again;
            int pressed_hard;
            int pressed_good;
            int pressed_easy;
            int time_spent;
            int cards_seen;
        } user;
    };

    StatsUpdateContext() : type(StatsUpdateType::Deck), deck{false, 0, 0} {}
    StatsUpdateContext(StatsUpdateType t) : type(t) {
        switch (t) {
            case StatsUpdateType::Card:
                card = {false, false, false, 0, 0, 0};
            break;
            case StatsUpdateType::Deck:
                deck = {false, 0, 0};
            break;
            case StatsUpdateType::User:
                user = {0, 0, 0, 0, 0, 0};
            break;
        }
    }
    ~StatsUpdateContext() = default;
};

#endif