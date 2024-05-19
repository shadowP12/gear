#pragma once

class Level;
class RenderScene;

class SceneRenderer
{
public:
    SceneRenderer();
    virtual ~SceneRenderer();

    void set_level(Level* level);

protected:
    RenderScene* scene;
};