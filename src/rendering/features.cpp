#include "features.h"

void FeatureConfig::get_shadow_macros(std::vector<std::string>& macros)
{
    if (shadow_mode == ShadowMode::Simple)
    {
        macros.push_back("SHADOW_SIMPLE");
    }
    else if (shadow_mode == ShadowMode::PCF)
    {
        if (pcf_method == PCFMethod::Optimized)
        {
            macros.push_back("SHADOW_OPTIMIZED_PCF");
        }
        else if (pcf_method == PCFMethod::RandomDisc)
        {
            macros.push_back("SHADOW_RANDOM_DISC_PCF");
        }
        else
        {
            macros.push_back("SHADOW_PCF");
        }
    }
    else if (shadow_mode == ShadowMode::VSM)
    {
        macros.push_back("SHADOW_VSM");
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

std::vector<Feature> FeatureConfig::get_diff_features(const FeatureConfig& config)
{
    std::vector<Feature> diff_features;
    if (shadow_mode != config.shadow_mode)
    {
        diff_features.push_back(Feature::Shadow);
    }
    else if (shadow_mode == ShadowMode::PCF)
    {
        if (pcf_method != config.pcf_method)
        {
            diff_features.push_back(Feature::Shadow);
        }
    }

    return diff_features;
}

FeatureConfig g_feature_config = FeatureConfig();