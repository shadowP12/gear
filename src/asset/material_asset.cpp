#include "material_asset.h"
#include "texture_asset.h"
#include "asset_manager.h"
#include "rendering/program.h"
#include "rendering/render_shared_data.h"
#include <core/memory.h>

MaterialAsset::MaterialAsset(const std::string& asset_path)
    : Asset(asset_path)
{
}

MaterialAsset::~MaterialAsset()
{
    for (auto& iter : _programs)
    {
        SAFE_DELETE(iter.second);
    }
    _programs.clear();
}

void MaterialAsset::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("passes");
    writer.StartArray();
    for (auto& pass_iter : _passes)
    {
        DrawType draw_type = pass_iter.first;
        Pass& pass = pass_iter.second;

        writer.StartObject();
        writer.Key("draw_type");
        writer.Int((int)draw_type);
        writer.Key("vs");
        writer.String(pass.vs.c_str());
        writer.Key("fs");
        writer.String(pass.fs.c_str());

        writer.Key("macros");
        writer.StartArray();
        for (auto& macro : pass.macros)
        {
            writer.String(macro.c_str());
        }
        writer.EndArray();

        writer.Key("params");
        writer.StartArray();
        for (auto& param_iter : pass.float_params)
        {
            writer.StartObject();
            writer.Key("param_name");
            writer.String(param_iter.first.c_str());
            writer.Key("param_type");
            writer.String("float");
            writer.Key("param_value");
            writer.Double(param_iter.second);
            writer.EndObject();
        }
        for (auto& param_iter : pass.vec2_params)
        {
            writer.StartObject();
            writer.Key("param_name");
            writer.String(param_iter.first.c_str());
            writer.Key("param_type");
            writer.String("float");
            writer.Key("param_value");
            Serialization::w_vec2(writer, param_iter.second);
            writer.EndObject();
        }
        for (auto& param_iter : pass.vec3_params)
        {
            writer.StartObject();
            writer.Key("param_name");
            writer.String(param_iter.first.c_str());
            writer.Key("param_type");
            writer.String("float");
            writer.Key("param_value");
            Serialization::w_vec3(writer, param_iter.second);
            writer.EndObject();
        }
        for (auto& param_iter : pass.vec4_params)
        {
            writer.StartObject();
            writer.Key("param_name");
            writer.String(param_iter.first.c_str());
            writer.Key("param_type");
            writer.String("float");
            writer.Key("param_value");
            Serialization::w_vec4(writer, param_iter.second);
            writer.EndObject();
        }
        for (auto& param_iter : pass.texture_params)
        {
            writer.StartObject();
            writer.Key("param_name");
            writer.String(param_iter.first.c_str());
            writer.Key("param_type");
            writer.String("float");
            writer.Key("param_value");
            writer.String(param_iter.second->get_asset_path().c_str());
            writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
}

void MaterialAsset::deserialize(const rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    for(int i = 0; i < value["passes"].Size(); i++)
    {
        const rapidjson::Value& pass_value = value["passes"][i];
        DrawType draw_type = (DrawType)pass_value["draw_type"].GetInt();

        Pass pass;
        Program* program = nullptr;

        pass.vs = pass_value["vs"].GetString();
        pass.fs = pass_value["fs"].GetString();

        if (pass_value.HasMember("macros"))
        {
            for (int j = 0; j < pass_value["macros"].Size(); j++)
            {
                pass.macros.push_back(pass_value["macros"][j].GetString());
            }
        }
        program = new Program(pass.vs, pass.fs, pass.macros);

        if (pass_value.HasMember("params"))
        {
            for (int j = 0; j < pass_value["params"].Size(); j++)
            {
                const rapidjson::Value& param_value = pass_value["params"][j];
                std::string param_name = pass_value["param_name"].GetString();
                std::string param_type = pass_value["param_type"].GetString();

                void* f = nullptr;
                EzTexture t = VK_NULL_HANDLE;
                if (param_type == "float")
                {
                    pass.float_params[param_name] = pass_value["param_value"].GetDouble();
                    f = &pass.float_params[param_name];
                }
                else if (param_type == "vec2")
                {
                    pass.vec2_params[param_name] = Serialization::r_vec2(pass_value["param_value"]);
                    f = &pass.vec2_params[param_name];
                }
                else if (param_type == "vec3")
                {
                    pass.vec3_params[param_name] = Serialization::r_vec3(pass_value["param_value"]);
                    f = &pass.vec3_params[param_name];
                }
                else if (param_type == "vec4")
                {
                    pass.vec4_params[param_name] = Serialization::r_vec4(pass_value["param_value"]);
                    f = &pass.vec4_params[param_name];
                }
                else if (param_type == "texture")
                {
                    pass.texture_params[param_name] = AssetManager::get()->load<TextureAsset>(pass_value["param_value"].GetString());
                    t = pass.texture_params[param_name]->get_texture();
                }

                if (f)
                {
                    program->set_parameter(param_name, f);
                }
                else if (t)
                {
                    program->set_parameter(param_name, t, g_rsd->get_sampler(SamplerType::LinearClamp), 0);
                }
            }
        }

        _passes[draw_type] = pass;
        _programs[draw_type] = program;
    }
}