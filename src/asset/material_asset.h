#pragma once

#include "asset.h"
#include "rendering/draw_command.h"

#include <unordered_map>
#include <vector>

class Program;
class TextureAsset;

class MaterialParam
{
public:
    enum class Type : int {
        Float,
        Vec2,
        Vec3,
        Vec4,
        Texture
    };

    void serialize(SerializationContext& ctx);

    void deserialize(DeserializationContext& ctx);

    Type type;
    std::string name;
    union {
        float float_val;
        glm::vec2 vec2_val;
        glm::vec3 vec3_val;
        glm::vec4 vec4_val;
        TextureAsset* texture_val;
    };
};

struct Pass {
    std::string vs;
    std::string fs;
    DrawType draw_type;
    std::vector<std::string> macros;
    std::vector<MaterialParam> params;
};

class MaterialAsset : public Asset
{
public:
    MaterialAsset(const std::string& asset_path = "");

    virtual ~MaterialAsset();

    virtual void serialize(SerializationContext& ctx, BinaryStream& bin_stream);

    virtual void deserialize(DeserializationContext& ctx, BinaryStream& bin_stream);

    const std::unordered_map<DrawType, Program*>& get_programs() { return _programs; }

private:
    std::unordered_map<DrawType, Pass> _passes;
    std::unordered_map<DrawType, Program*> _programs;
};