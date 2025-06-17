#include "level.h"
#include "entity/components/c_camera.h"
#include "entity/entity.h"

Level::Level(const std::string& asset_path)
    : Asset(asset_path) {}

Level::~Level()
{
    for (size_t i = 0; i < _entities.size(); ++i)
    {
        delete _entities[i];
        _entities[i] = nullptr;
    }
    _entities.clear();
}

void Level::serialize(SerializationContext& ctx, BinaryStream& bin_stream)
{
    std::vector<int> hierarchy;
    hierarchy.resize(_entities.size());

    ctx.object([&]() {
        ctx.array("entities", [&]() {
            for (size_t i = 0; i < _entities.size(); ++i)
            {
                hierarchy[i] = -1;
                Entity* parent = _entities[i]->get_parent();
                if (parent && parent->get_level_id() >= 0)
                {
                    hierarchy[i] = parent->get_level_id();
                }
                _entities[i]->serialize(ctx, bin_stream);
            }
        });

        ctx.array("hierarchy", [&]() {
            for (size_t i = 0; i < hierarchy.size(); ++i)
            {
                if (hierarchy[i] >= 0)
                {
                    ctx.array("hierarchy", [&]() {
                        ctx.field(i);
                        ctx.field(hierarchy[i]);
                    });
                }
            }
        });
    });
}

void Level::deserialize(DeserializationContext& ctx, BinaryStream& bin_stream)
{
    int level_id = 0;
    ctx.array("entities", [&]() {
        Entity* entity = new Entity();
        entity->set_level_id(level_id++);
        entity->deserialize(ctx, bin_stream);
        _entities.push_back(entity);
    });

    if (ctx.has_key("hierarchy"))
    {
        ctx.array("hierarchy", [&]() {
            int child_idx;
            ctx.field(0, child_idx);
            int parent_idx;
            ctx.field(1, parent_idx);
            Entity* entity = _entities[child_idx];
            Entity* parent = _entities[parent_idx];
            entity->set_parent(parent);
        });
    }
}