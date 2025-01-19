//
// Created by TehPig on 1/13/2025.
//

#ifndef SM2_HPP
#define SM2_HPP

#include "Backend/Classes/Base/Algorithm.hpp"
#include "Backend/Classes/Stats/CardStats.hpp"

class SM2Algorithm final : public Algorithm {
public:
    void calculateInterval(CardStats& stats, int buttonPressed) override;

};

#endif