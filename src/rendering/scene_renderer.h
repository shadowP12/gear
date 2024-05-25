#pragma once

class Level;
class RenderScene;
class RenderContext;
class SceneRenderer
{
public:
    SceneRenderer();
    virtual ~SceneRenderer();

    void set_level(Level* level);

    virtual void render(RenderContext* ctx);

protected:
    RenderScene* scene;
};