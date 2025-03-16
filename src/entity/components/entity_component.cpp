#include "entity_component.h"
#include "entity/entity.h"

EntityComponent::EntityComponent(Entity* entity)
{
    _entity = entity;
    if (listen_transform_changed())
    {
        _transform_changed_event_handle = _entity->transform_changed_event.bind(EVENT_CB(notify_transform_changed));
    }
}

EntityComponent::~EntityComponent()
{
    if (listen_transform_changed())
    {
        _entity->transform_changed_event.unbind(_transform_changed_event_handle);
    }
}