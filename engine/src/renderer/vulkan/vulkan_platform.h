#pragma once

#include "defines.h"

struct platform_state;
struct vulkan_context;

b8 platform_create_vulkan_surface(struct vulkan_context* context);


/**
 * Appends the nmes of required extensions for this platform.
 * To the names_darray, which should be created nad passed in.
 */
void platform_get_required_extension_names(const char*** names_darray);