#pragma once

#include "vulkan_types.inl"

void vulkan_frame_buffer_create(
    vulkan_context* context,
    vulkan_render_pass* render_pass,
    u32 width,
    u32 height,
    u32 attachment_count,
    VkImageView* attachments,
    vulkan_frame_buffer* out_frame_buffer
);

void vulkan_frame_buffer_destroy(vulkan_context* context, vulkan_frame_buffer* frame_buffer);