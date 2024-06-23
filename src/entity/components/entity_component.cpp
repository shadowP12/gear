#include "entity_component.h"
#include "entity/entity.h"
#include "entity/entity_notifications.h"

EntityComponent::EntityComponent(Entity* entity)
{
    _entity = entity;
}

EntityComponent::~EntityComponent()
{
}