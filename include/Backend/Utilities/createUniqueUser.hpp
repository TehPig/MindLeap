//
// Created by TehPig on 1/7/2025.
//

#ifndef CREATEUNIQUEUSER_HPP
#define CREATEUNIQUEUSER_HPP

#include <QString>

#include "Backend/Classes/Stats/UserStats.hpp"

QString createUniqueUser(const QString &username, UserStats &stats);

#endif