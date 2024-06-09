#include "material_proxy.h"

void MaterialProxy::make_dirty()
{
    if (!dirty)
        dirty = true;
}

void MaterialProxy::clear_dirty()
{
    dirty = false;
}

void MaterialProxy::modify_compilation_environment(std::vector<std::string>& macros)
{
    if (base_color_texture)
    {
        macros.push_back("USING_BASE_COLOR_TEXTURE");
    }
}

MaterialProxyPool::MaterialProxyPool()
{
    _proxys.clear();
}

MaterialProxyPool::~MaterialProxyPool()
{
    _proxys.clear();
}

int MaterialProxyPool::register_proxy()
{
    int material_id = _proxys.size();
    _proxys.emplace_back();
    return material_id;
}

void MaterialProxyPool::unregister_proxy(int material_id)
{
    MaterialProxy* proxy = get_proxy(material_id);
    proxy->clear_dirty();
    proxy->material_ub.reset();
}

MaterialProxy* MaterialProxyPool::get_proxy(int material_id)
{
    return &_proxys[material_id];
}

void MaterialProxyPool::update_dirty_proxys()
{
    for (int i = 0; i < _proxys.size(); ++i)
    {
        MaterialProxy* proxy = &_proxys[i];
        if (proxy->dirty || !proxy->material_ub)
        {
            proxy->clear_dirty();

            if (!proxy->material_ub)
            {
                proxy->material_ub = std::make_shared<UniformBuffer>(sizeof(MaterialParams));
            }
            proxy->material_ub->write((uint8_t*)&proxy->params, sizeof(MaterialParams));
        }
    }
}