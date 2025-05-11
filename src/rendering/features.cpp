#include "features.h"

void FeatureConfig::get_shadow_macros(std::vector<std::string>& macros)
{
    macros.push_back("SHADOW_FEATURE");
    if (shadow_mode == ShadowMode::Simple)
    {
        macros.push_back("SHADOW_MODE_SIMPLE");
    }
    else if(shadow_mode == ShadowMode::PCF)
    {
        macros.push_back("SHADOW_MODE_PCF");
    }
}

void FeatureConfig::get_macros(const std::vector<Feature>& features, std::vector<std::string>& macros)
{
    for (int i = 0; i < features.size(); ++i)
    {
        if (features[i] == Feature::Shadow)
        {
            get_shadow_macros(macros);
        }
    }
}

FeatureConfig g_feature_config = FeatureConfig();