//
// Created by TehPig on 1/13/2025.
//

#include "Backend/Classes/Stats/CardStats.hpp"
#include "Backend/Classes/Algorithms/SM2.hpp"

void SM2Algorithm::calculateInterval(CardStats& stats, int buttonPressed) {
    const double MIN_EASE_FACTOR = 1.3;
    const int SECONDS_IN_A_DAY = 86400;

    switch (buttonPressed) {
        case 1: // "Again"
            stats.setRepetitions(0);
            stats.setEaseFactor(2.5);
            stats.setInterval(60); // 1 minute (60 seconds)
            break;
        case 2: // "Hard"
            stats.setEaseFactor(std::max(MIN_EASE_FACTOR, stats.getEaseFactor() - 0.15));
            stats.setInterval((stats.getRepetitions() == 0) ? 360 : static_cast<int>(stats.getInterval() * 1.2)); // 6 minutes (360 seconds) or scaled
            break;
        case 3: // "Good"
            stats.setRepetitions(stats.getRepetitions() + 1);
            if (stats.getRepetitions() == 1) {
                stats.setInterval(600); // 10 minutes (600 seconds)
            } else {
                stats.setInterval(static_cast<int>(stats.getInterval() * stats.getEaseFactor()));
            }
            break;
        case 4: // "Easy"
            stats.setRepetitions(stats.getRepetitions() + 1);
            stats.setInterval(stats.getRepetitions() == 1 ? SECONDS_IN_A_DAY * 5 : static_cast<int>(stats.getInterval() * stats.getEaseFactor() * 1.3)); // 5 days or scaled
            stats.setEaseFactor(stats.getEaseFactor() + 0.15);
            break;

        // If no valid button number is specified, print an error message
        default:
            qDebug() << "[SM2] Invalid button pressed.";
            break;
    }
}