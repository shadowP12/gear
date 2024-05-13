#include "entity_component.h"
#include "../entity.h"

EntityComponent::EntityComponent(Entity* entity)
{
    _entity = entity;
}

EntityComponent::~EntityComponent()
{
}