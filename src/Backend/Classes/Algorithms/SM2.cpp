//
// Created by TehPig on 1/13/2025.
//

#include "Backend/Classes/Algorithms/SM2.hpp"

void SM2Algorithm::calculateInterval(Card& card, int buttonPressed) {
    const double MIN_EASE_FACTOR = 1.3;

    switch (buttonPressed) {
        case 1: // "Again"
            card.setRepetitions(0);
            card.setEaseFactor(2.5);
            card.setInterval(0.0167); // 1 minute (0.0167 days)
            break;

        case 2: // "Hard"
            card.setEaseFactor(std::max(MIN_EASE_FACTOR, card.getEaseFactor() - 0.15));
            card.setInterval((card.getRepetitions() == 0) ? 0.0042 : card.getInterval() * 1.2); // 6 minutes or scaled
            break;

        case 3: // "Good"
            card.setRepetitions(card.getRepetitions() + 1);
            if (card.getRepetitions() == 1) {
                card.setInterval(0.0069); // 10 minutes (0.0069 days)
            } else {
                card.setInterval(card.getInterval() * card.getEaseFactor());
            }
            break;

        case 4: // "Easy"
            card.setRepetitions(card.getRepetitions() + 1);
            card.setInterval(card.getRepetitions() == 1 ? 5 : card.getInterval() * card.getEaseFactor() * 1.3); // 5 days or scaled
            card.setEaseFactor(card.getEaseFactor() + 0.15);
            break;

        default:
            qDebug() << "[Study - Card Error] Invalid button pressed.";
            break;
    }
}