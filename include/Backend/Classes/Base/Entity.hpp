//
// Created by TehPig on 1/5/2025.
//

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <QString>

class Entity {
protected:
    void logAction(const QString& action) const;
    bool validateID(const QString& id) const;

public:
    virtual ~Entity() = default;

    virtual bool create() = 0;
    virtual bool _delete() const = 0;
    virtual void getStats() const = 0;
};

#endif
