#include "c_render.h"
#include "../../rendering/render_system.h"

CRender::CRender(Entity* entity)
    : EntityComponent(entity)
{
    _predraw_event_handle = RenderSystem::get()->predraw_event.bind(EVENT_CB(predraw_imp));
}

CRender::~CRender()
{
    RenderSystem::get()->predraw_event.unbind(_predraw_event_handle);
}

void CRender::notify_transform_changed()
{
    make_render_dirty();
}

void CRender::predraw_imp()
{
    if (_render_dirty)
    {
        predraw();
    }
    _render_dirty = false;
}

REGISTER_ENTITY_COMPONENT(CRender)