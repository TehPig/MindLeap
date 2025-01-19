//
// Created by TehPig on 1/5/2025.
//

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <QString>
#include <QDebug>

class Entity {
protected:
    static void logAction(const QString& action) {
        qDebug() << "====================================================";
        qDebug() << "[Entity] Action performed:" << action;
    }

public:
    virtual ~Entity() = default;

    virtual bool create() = 0;
    virtual bool _delete() const = 0;
    virtual void getStats() const = 0;
};

#endif
