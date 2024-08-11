#pragma once

#include "base_collector.h"
#include "rendering/renderable.h"

class RenderableCollector : public BaseCollector<Renderable>
{
};

class SceneInstanceCollector : public BaseCollector<SceneInstanceData>
{
};