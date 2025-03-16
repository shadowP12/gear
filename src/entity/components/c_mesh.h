#pragma once

#include "c_render.h"
#include "rendering/vertex_factory.h"
#include <core/enum_flag.h>

class Entity;
class MeshAsset;

enum class MeshRnderDirtyFlag
{
    None = 0
};
SP_MAKE_ENUM_FLAG(uint32_t, MeshRnderDirtyFlag)

class CMesh : public CRender
{
public:
    CMesh(Entity* entity);

    virtual ~CMesh();

    static std::string get_static_class_name() { return "CMesh"; }

    virtual std::string get_class_name() { return "CMesh"; }

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);

    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    void set_mesh(MeshAsset* mesh);

    MeshAsset* get_mesh();

protected:
    void make_mesh_render_dirty(MeshRnderDirtyFlag flag);

    void destroy_renderables();

    void predraw() override;

private:
    MeshAsset* _mesh;
    std::vector<VertexFactory> _vertex_factorys;
    std::vector<uint32_t> _renderables;
    MeshRnderDirtyFlag _render_dirty_flags = MeshRnderDirtyFlag::None;
};