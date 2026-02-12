//
// Created by TehPig on 1/13/2025.
//

#include "Backend/Classes/Stats/CardStats.hpp"
#include "Backend/Classes/Algorithms/SM2.hpp"
#include "Backend/Utilities/Logger.hpp"

void SM2Algorithm::calculateInterval(CardStats& stats, int buttonPressed) {
    const double MIN_EASE_FACTOR = 1.3;

    switch (buttonPressed) {
        case 1: // "Again"
            stats.setRepetitions(0);
            stats.setEaseFactor(2.5);
            stats.setInterval(0); // Due immediately
            break;
        case 2: // "Hard"
            stats.setEaseFactor(std::max(MIN_EASE_FACTOR, stats.getEaseFactor() - 0.15));
            // If it's the first time, keep it at 0 to see it again soon. Otherwise, scale by 1.2x days.
            stats.setInterval((stats.getRepetitions() == 0) ? 0 : static_cast<int>(stats.getInterval() * 1.2));
            break;
        case 3: // "Good"
            stats.setRepetitions(stats.getRepetitions() + 1);
            if (stats.getRepetitions() == 1) {
                stats.setInterval(1); // 1 day
            } else {
                stats.setInterval(std::max(1, static_cast<int>(stats.getInterval() * stats.getEaseFactor())));
            }
            break;
        case 4: // "Easy"
            stats.setRepetitions(stats.getRepetitions() + 1);
            // 4 days for the first time, then scale
            stats.setInterval(std::max(1, stats.getRepetitions() == 1 ? 4 : static_cast<int>(stats.getInterval() * stats.getEaseFactor() * 1.3)));
            stats.setEaseFactor(stats.getEaseFactor() + 0.15);
            break;

        default:
            Logger::warn("Invalid button pressed", "SM2");
            break;
    }

    Logger::info(QString("Calculated Interval: %1 days (Ease: %2, Reps: %3)")
                 .arg(QString::number(stats.getInterval()), QString::number(stats.getEaseFactor()), QString::number(stats.getRepetitions())), "SM2");
}