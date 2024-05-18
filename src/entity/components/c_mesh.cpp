#include "c_mesh.h"
#include "entity/entity.h"
#include "asset/mesh.h"
#include "asset/asset_manager.h"

CMesh::CMesh(Entity* entity)
    : EntityComponent(entity)
{
}

CMesh::~CMesh()
{
}

void CMesh::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Serialization::BinaryStream& bin)
{
    writer.StartObject();
    writer.Key("class_name");
    writer.String(get_class_name().c_str());
    writer.Key("mesh");
    writer.String(_mesh->get_asset_path().c_str());
    writer.EndObject();
}

void CMesh::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
    _mesh = AssetManager::get()->load<Mesh>(value["mesh"].GetString());
}

void CMesh::set_mesh(Mesh* mesh) {
    _mesh = mesh;
}

Mesh* CMesh::get_mesh() {
    return _mesh;
}

REGISTER_ENTITY_COMPONENT(CMesh)