//
// Created by wirewhiz on 27/07/22.
//

#include "entityID.h"
EntityID& EntityID::operator=(uint32_t value) {
    id = value;
    return *this;
}

EntityID::operator uint32_t() const{
    return id;
}