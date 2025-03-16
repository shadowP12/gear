#pragma once

#include <core/event.h>
#include "entity_component.h"

class Entity;
class CRender : public EntityComponent
{
public:
    CRender(Entity* entity);

    virtual ~CRender();

    static std::string get_static_class_name() { return "CRender"; }

    virtual std::string get_class_name() { return "CRender"; }

    virtual void notify_transform_changed();

protected:
    virtual void predraw() {};

    virtual void make_render_dirty() { _render_dirty = true; };

protected:
    void predraw_imp();

private:
    bool _render_dirty = true;
    EventHandle _predraw_event_handle;
};