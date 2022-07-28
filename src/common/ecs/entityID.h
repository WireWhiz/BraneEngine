//
// Created by wirewhiz on 27/07/22.
//

#ifndef BRANEENGINE_ENTITYID_H
#define BRANEENGINE_ENTITYID_H

#include <cstdint>

struct EntityID
{
    uint32_t id = 0;
    uint32_t version = -1;
    operator uint32_t() const;
    EntityID& operator=(uint32_t);
};


#endif //BRANEENGINE_ENTITYID_H
