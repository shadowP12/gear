#pragma once

#include <rhi/ez_vulkan.h>

EzBuffer create_render_buffer(const EzBufferDesc& desc, EzResourceState dst_state, void* data = nullptr);

void clear_render_buffer(EzBuffer buffer, EzResourceState dst_state);

void update_render_buffer(EzBuffer buffer, EzResourceState dst_state, uint32_t size, uint32_t offset, void* data);