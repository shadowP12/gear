#pragma once

#include "asset.h"
#include "rendering/draw_command.h"
#include <vector>
#include <unordered_map>

class Program;
class TextureAsset;

class MaterialAsset : public Asset
{
public:
    MaterialAsset(const std::string& asset_path = "");

    virtual ~MaterialAsset();

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);

    virtual void deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin);

    const std::unordered_map<DrawType, Program*>& get_programs() { return _programs; }

private:
    struct Pass
    {
        std::string vs;
        std::string fs;
        std::vector<std::string> macros;
        std::unordered_map<std::string, float> float_params;
        std::unordered_map<std::string, glm::vec2> vec2_params;
        std::unordered_map<std::string, glm::vec3> vec3_params;
        std::unordered_map<std::string, glm::vec4> vec4_params;
        std::unordered_map<std::string, TextureAsset*> texture_params;
    };
    std::unordered_map<DrawType, Pass> _passes;
    std::unordered_map<DrawType, Program*> _programs;
};