#include "material_asset.h"

#include "asset_manager.h"
#include "rendering/program.h"
#include "rendering/render_shared_data.h"
#include "texture_asset.h"

#include <core/memory.h>

void MaterialParam::serialize(SerializationContext& ctx)
{
    ctx.field("name", name);
    ctx.field("type", type);
    ctx.key("value");
    switch (type)
    {
        case Type::Float:
            ctx.field(float_val);
            break;
        case Type::Vec2:
            ctx.field(vec2_val);
            break;
        case Type::Vec3:
            ctx.field(vec3_val);
            break;
        case Type::Vec4:
            ctx.field(vec4_val);
            break;
        case Type::Texture:
            ctx.field(texture_val->get_asset_path());
            break;
        default:
            break;
    }
}

void MaterialParam::deserialize(DeserializationContext& ctx)
{
    ctx.field("name", name);
    ctx.field("type", type);
    switch (type)
    {
        case Type::Float:
            ctx.field("value", float_val);
            break;
        case Type::Vec2:
            ctx.field("value", vec2_val);
            break;
        case Type::Vec3:
            ctx.field("value", vec3_val);
            break;
        case Type::Vec4:
            ctx.field("value", vec4_val);
            break;
        case Type::Texture: {
            std::string asset_path;
            ctx.field("value", asset_path);
            texture_val = AssetManager::get()->load<TextureAsset>(asset_path);
            break;
        }
        default:
            break;
    }
}

MaterialAsset::MaterialAsset(const std::string& asset_path)
    : Asset(asset_path) {}

MaterialAsset::~MaterialAsset()
{
    for (auto& iter : _programs)
    {
        SAFE_DELETE(iter.second);
    }
    _programs.clear();
}

void MaterialAsset::serialize(SerializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.object([&]() {
        ctx.array("passes", [&]() {
            for (auto& pass_it : _passes)
            {
                auto& pass = pass_it.second;
                ctx.object([&]() {
                    ctx.field("draw_type", pass.draw_type);
                    ctx.field("vs", pass.vs);
                    ctx.field("fs", pass.fs);

                    ctx.array("macros", [&]() {
                        for (const auto& macro : pass.macros)
                        {
                            ctx.field(macro);
                        }
                    });

                    ctx.array("params", [&]() {
                        for (MaterialParam& param : pass.params)
                        {
                            param.serialize(ctx);
                        }
                    });
                });
            }
        });
    });
}

void MaterialAsset::deserialize(DeserializationContext& ctx, BinaryStream& bin_stream)
{
    ctx.array("passes", [&]() {
        Pass pass;
        ctx.field("draw_type", pass.draw_type);
        ctx.field("vs", pass.vs);
        ctx.field("fs", pass.fs);

        uint32_t macro_idx = 0;
        ctx.array("macros", [&]() {
            std::string macro;
            ctx.field(macro_idx++, macro);
            pass.macros.push_back(macro);
        });

        std::vector<Feature> features;
        features.push_back(Feature::Shadow);

        ProgramDesc program_desc;
        program_desc.vs = pass.vs;
        program_desc.fs = pass.fs;
        program_desc.macros = pass.macros;
        program_desc.features = features;
        Program* program = new Program(program_desc);

        ctx.array("params", [&]() {
            MaterialParam param;
            param.deserialize(ctx);
            pass.params.push_back(param);

            if (param.type == MaterialParam::Type::Texture)
            {
                program->set_parameter(param.name, param.texture_val->get_texture(), g_rsd->get_sampler(SamplerType::LinearClamp), 0);
            }
            else
            {
                program->set_parameter(param.name, &param.float_val);
            }
        });

        _passes[pass.draw_type] = pass;
        _programs[pass.draw_type] = program;
    });
}