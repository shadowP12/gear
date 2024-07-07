#pragma once

#include <core/event.h>
#include <vector>

class Viewport;
class Level;
class Entity;

class World
{
public:
    World();
    ~World();

    void set_viewport(Viewport* vp);

    Viewport* get_viewport();

    void add_level(Level* level);

    void tick(float dt);

    Entity* get_entity(int idx) { return _entities[idx]; }

    std::vector<Entity*>& get_entities() { return _entities; }

    std::vector<Entity*>& get_mesh_entities() { return _mesh_entities; };

    std::vector<Entity*>& get_camera_entities() { return _camera_entities; };

    std::vector<Entity*>& get_light_entities() { return _light_entities; };

private:
    Viewport* _vp = nullptr;
    std::vector<Entity*> _entities;
    std::vector<Entity*> _mesh_entities;
    std::vector<Entity*> _camera_entities;
    std::vector<Entity*> _light_entities;
};