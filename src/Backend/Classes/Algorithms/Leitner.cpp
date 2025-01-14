//
// Created by TehPig on 1/13/2025.
//

#include "Backend/Classes/Algorithms/Leitner.hpp"

void LeitnerAlgorithm::calculateInterval(Card& card, int buttonPressed) {
    switch (buttonPressed) {
        case 1: // Again
            card.setInterval(0.0);
            break;
        case 2: // Hard
            card.setInterval(card.getInterval() * 1.2);
            break;
        case 3: // Good
            card.setInterval(card.getInterval() * 1.5);
            break;
        case 4: // Easy
            card.setInterval(card.getInterval() * 2.0);
            break;
        default:
            qDebug() << "Invalid button pressed.";
            break;
    }
}