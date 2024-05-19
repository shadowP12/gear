#pragma once

class Level;
class Entity;

class RenderScene
{
public:
    RenderScene();
    ~RenderScene();

    void set_level(Level* level);

private:
    Level* _level;
};