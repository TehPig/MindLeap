//
// Created by TehPig on 1/13/2025.
//

#include "Backend/Classes/Stats/CardStats.hpp"
#include "Backend/Classes/Algorithms/Leitner.hpp"

void LeitnerAlgorithm::calculateInterval(CardStats& stats, int buttonPressed) {
    switch (buttonPressed) {
        case 1: // Again
            stats.setInterval(stats.getInterval() * 0.3);
            break;
        case 2: // Hard
            stats.setInterval(stats.getInterval() * 0.6);
            break;
        case 3: // Good
            stats.setInterval(stats.getInterval() * 1.7);
            break;
        case 4: // Easy
            stats.setInterval(stats.getInterval() * 2.0);
            break;

        // If no valid button number is specified, print an error message
        default:
            qDebug() << "[Leitner] Invalid button pressed.";
            break;
    }
}