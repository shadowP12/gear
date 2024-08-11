#pragma once

#include "entity_component.h"

class Entity;
class Mesh;
class RenderableCollector;
class SceneInstanceCollector;
class CMesh : public EntityComponent
{
public:
    CMesh(Entity* entity);

    virtual ~CMesh();

    static std::string get_static_class_name() { return "CMesh"; }
    virtual std::string get_class_name() { return "CMesh"; }

    virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin);
    virtual void deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin);

    void set_mesh(Mesh* mesh);

    Mesh* get_mesh();

    void fill_renderables(RenderableCollector* collector, SceneInstanceCollector* scene_collector);

private:
    Mesh* _mesh;
};