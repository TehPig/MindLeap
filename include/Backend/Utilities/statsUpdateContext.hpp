//
// Created by TehPig on 1/5/2025.
//

#ifndef STATSUPDATECONTEXT_HPP
#define STATSUPDATECONTEXT_HPP

#include <QString>
#include <QDateTime>

struct StatsUpdateContext {
    int button_pressed; // Button pressed by user
    int time_spent; // Time spent on card/deck
    QDateTime last_seen; // Timestamp of last interaction
    //QString extra_info; // Additional info if needed
};

#endif