//
// Created by TehPig on 1/13/2025.
//

#ifndef LEITNER_HPP
#define LEITNER_HPP

#include "Backend/Classes/Base/Algorithm.hpp"
#include "Backend/Classes/Stats/CardStats.hpp"

class LeitnerAlgorithm final : public Algorithm {
public:
    void calculateInterval(CardStats& stats, int buttonPressed) override;

};

#endif