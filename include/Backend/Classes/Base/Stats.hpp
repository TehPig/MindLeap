//
// Created by TehPig on 1/5/2025.
//

#ifndef STATS_HPP
#define STATS_HPP

#include "Backend/Utilities/statsUpdateContext.hpp"

class Stats {
public:
    virtual ~Stats() = default;

    virtual void updateStats(const StatsUpdateContext& context) = 0;
    virtual void displayStats() const = 0;
};

#endif
