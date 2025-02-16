#include "material_proxy.h"
#include <core/memory.h>

static int g_material_id = 0;

MaterialProxy::MaterialProxy()
{
    material_id = g_material_id++;
}

MaterialProxy::~MaterialProxy()
{
    if (material_ub)
    {
        SAFE_DELETE(material_ub);
    }
}

void MaterialProxy::make_dirty()
{
    if (!dirty)
        dirty = true;
}

void MaterialProxy::clear_dirty()
{
    if (dirty || !material_ub)
    {
        if (!material_ub)
        {
            material_ub = new GpuBuffer(BufferUsageFlags::Dynamic | BufferUsageFlags::Uniform, sizeof(MaterialParams));
        }
        material_ub->write((uint8_t*)&params, sizeof(MaterialParams));
    }
    dirty = false;
}

void MaterialProxy::compilation_environment(std::vector<std::string>& macros)
{
    if (base_color_texture)
    {
        macros.push_back("USING_BASE_COLOR_TEXTURE");
    }
}

void MaterialProxy::bind()
{
    ez_bind_buffer(2, material_ub->get_handle());

    if (base_color_texture)
    {
        ez_bind_texture(10, base_color_texture, 0);
    }
}