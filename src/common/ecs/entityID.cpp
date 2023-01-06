//
// Created by wirewhiz on 27/07/22.
//

#include "entityID.h"
EntityID &EntityID::operator=(uint32_t value)
{
  id = value;
  return *this;
}

bool EntityID::operator==(const EntityID &o) const { return id == o.id && version == o.version; }

bool EntityID::operator!=(const EntityID &o) const { return !(*this == o); }
