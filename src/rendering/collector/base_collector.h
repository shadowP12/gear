#pragma once

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
    }

    T* get_data()
    {
        return items.data();
    }

    int get_size()
    {
        return count * sizeof(T);
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

public:
    int count = 0;
    std::vector<T> items;
};