//
// Created by TehPig on 1/13/2025.
//

#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include "Backend/Classes/Stats/CardStats.hpp"

class Algorithm {
public:
    virtual void calculateInterval(CardStats& card, int buttonPressed) = 0;
    virtual ~Algorithm() = default;
};

#endif