//
// Created by TehPig on 1/5/2025.
//

#include <QDebug>

#include "Backend/Classes/Base/Entity.hpp"

// Protected methods
void Entity::logAction(const QString& action) const {
    qDebug() << "[Entity] Action performed:" << action;
}

bool Entity::validateID(const QString& id) const {
    return !id.isEmpty();
}

// Public methods
bool Entity::create() {
    qDebug() << "Default create method called.";
    return true;
}

bool Entity::_delete() const {
    qDebug() << "Default delete method called.";
    return true;
}

void Entity::getStats() const {
    qDebug() << "Default getStats called.";
}
