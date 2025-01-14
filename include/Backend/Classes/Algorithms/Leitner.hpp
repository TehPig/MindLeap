//
// Created by TehPig on 1/13/2025.
//

#ifndef LEITNER_HPP
#define LEITNER_HPP

#include "Backend/Classes/Base/Algorithm.hpp"
#include "Backend/Classes/Card.hpp"

class LeitnerAlgorithm : public Algorithm {
public:
    static void calculateInterval(Card& card, int buttonPressed) override;

};

#endif