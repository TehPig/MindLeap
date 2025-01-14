//
// Created by TehPig on 1/13/2025.
//

#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include "Backend/Classes/Card.hpp"

class Algorithm {
public:
    virtual void calculateInterval(Card& card, int buttonPressed) = 0;
    virtual ~Algorithm() = default;
};

#endif