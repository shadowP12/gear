#include "world.h"
#include "asset/level.h"
#include "entity/components/c_camera.h"
#include "entity/components/c_light.h"
#include "entity/components/c_mesh.h"
#include "entity/entity.h"

World::World()
{
}

World::~World()
{
}

void World::set_viewport(Viewport* vp)
{
    _vp = vp;
}

Viewport* World::get_viewport()
{
    return _vp;
}

void World::add_level(Level* level)
{
    std::vector<Entity*>& entities = level->get_entities();
    for (auto entity : entities)
    {
        entity->set_world(this);
        _entities.push_back(entity);

        // Categories
        if (entity->has_component<CMesh>())
        {
            _mesh_entities.push_back(entity);
        }

        if (entity->has_component<CCamera>())
        {
            _camera_entities.push_back(entity);
        }

        if (entity->has_component<CLight>())
        {
            _light_entities.push_back(entity);
        }
    }
}

void World::tick(float dt)
{
}