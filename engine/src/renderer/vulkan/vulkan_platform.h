#pragma once

#include "defines.h"


/**
 * Appends the nmes of required extensions for this platform.
 * To the names_darray, which should be created nad passed in.
 */
void platform_get_required_extension_names(const char*** names_darray);