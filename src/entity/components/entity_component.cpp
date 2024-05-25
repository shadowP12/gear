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

void EntityComponent::make_dirty()
{
    if (!_dirty)
    {
        _dirty = true;
        _entity->notify.broadcast(NOTIFY_ENTITY_DIRTIED, _entity->get_id());
    }
}

void EntityComponent::dirty_notify()
{
    if (_dirty)
    {
        _dirty = false;
        dirty_notify_imp();
    }
}