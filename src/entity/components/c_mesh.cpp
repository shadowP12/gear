#include "c_mesh.h"
#include "../entity.h"

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
    writer.EndObject();
}

void CMesh::deserialize(rapidjson::Value& value, Serialization::BinaryStream& bin)
{
}

REGISTER_ENTITY_COMPONENT(CMesh)