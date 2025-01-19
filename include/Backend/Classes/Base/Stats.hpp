//
// Created by TehPig on 1/5/2025.
//

#ifndef STATS_HPP
#define STATS_HPP

#include "Backend/Utilities/statsUpdateContext.hpp"

class Stats {
public:
    virtual ~Stats() = default;

    virtual Stats* load() = 0;
    virtual bool initialize() const = 0;

    virtual bool update(const StatsUpdateContext& context) = 0;
    virtual void display() const = 0;
};

#endif
