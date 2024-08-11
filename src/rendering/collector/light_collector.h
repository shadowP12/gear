#pragma once

#include "base_collector.h"
#include "rendering/render_constants.h"

class OmniLightCollector : public BaseCollector<OmniLightData>
{
};

class DirectionLightCollector : public BaseCollector<DirectionLightData>
{
};