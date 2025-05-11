#pragma once
#include <string>
#include <vector>

enum class ShadowMode
{
    Simple = 0,
    PCF,
    Count
};

enum class Feature
{
    Shadow = 0,
    Count
};

class FeatureConfig
{
public:
    void get_macros(const std::vector<Feature>& features, std::vector<std::string>& macros);

    void get_shadow_macros(std::vector<std::string>& macros);

    ShadowMode shadow_mode;
};

extern FeatureConfig g_feature_config;