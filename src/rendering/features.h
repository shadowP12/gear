#pragma once
#include <string>
#include <vector>

enum class ShadowMode {
    Simple = 0,
    PCF,
    VSM
};

enum class PCFMethod {
    Common = 0,
    Optimized,
    RandomDisc
};

enum class Feature {
    Shadow = 0,
    Count
};

class FeatureConfig
{
public:
    std::vector<Feature> get_diff_features(const FeatureConfig& config);

    void get_macros(const std::vector<Feature>& features, std::vector<std::string>& macros);

    void get_shadow_macros(std::vector<std::string>& macros);

    ShadowMode shadow_mode = ShadowMode::VSM;
    PCFMethod pcf_method = PCFMethod::Optimized;
};

extern FeatureConfig g_feature_config;