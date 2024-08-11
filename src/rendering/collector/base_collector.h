#pragma once

#include "rendering/render_resources.h"
#include <core/memory.h>
#include <vector>

template<class T>
class BaseCollector
{
public:
    BaseCollector() {}

    ~BaseCollector()
    {
        count = 0;
        items.clear();
        if (ub)
        {
            SAFE_DELETE(ub);
        }
    }

    void clear()
    {
        count = 0;
    }

    virtual int add_item()
    {
        if (count >= items.size())
        {
            items.emplace_back();
        }

        return count++;
    }

    T* get_item(int idx)
    {
        return &items[idx];
    }

    virtual void update_ub()
    {
        if (count <= 0)
        {
            return;
        }

        if (ub && ub->get_buffer()->size < items.size() * sizeof(T))
        {
            SAFE_DELETE(ub);
        }

        if (!ub)
        {
            uint32_t buffer_size = count * sizeof(T);
            ub = new UniformBuffer(buffer_size);
        }

        ub->write((uint8_t*)items.data(), count * sizeof(T));
    }

public:
    int count = 0;
    std::vector<T> items;
    UniformBuffer* ub = nullptr;
};