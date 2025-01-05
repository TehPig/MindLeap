//
// Created by TehPig on 1/5/2025.
//

#include <QDebug>

#include "Backend/Utilities/statsUpdateContext.hpp"
#include "Backend/Classes/Base/Stats.hpp"

void Stats::updateStats(const StatsUpdateContext& context) {
    qDebug() << "Default updateStats called.";
}

void Stats::displayStats() const {
    qDebug() << "Default displayStats called.";
}

