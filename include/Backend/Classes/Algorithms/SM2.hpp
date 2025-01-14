//
// Created by TehPig on 1/13/2025.
//

#ifndef SM2_HPP
#define SM2_HPP

#include "Backend/Classes/Base/Algorithm.hpp"
#include "Backend/Classes/Card.hpp"

class SM2Algorithm : public Algorithm {
public:
    static void calculateInterval(Card& card, int buttonPressed) override;

};

#endif