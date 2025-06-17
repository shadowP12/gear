#pragma once

#include "asset.h"
#include <core/event.h>

class Entity;

class Level : public Asset
{
public:
    Level(const std::string& asset_path = "");

    virtual ~Level();

    virtual void serialize(SerializationContext& ctx, BinaryStream& bin_stream);

    virtual void deserialize(DeserializationContext& ctx, BinaryStream& bin_stream);

    std::vector<Entity*>& get_entities() { return _entities; }

protected:
    std::vector<Entity*> _entities;
};