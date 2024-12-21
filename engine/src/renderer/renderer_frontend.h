#pragma once

#include "renderer_types.inl"

/**
 * @brief Initializes renderer system. Call twice; once with state = 0 to get required memory size,
 * then a second time passing allocated memory to state.
 * 
 * @param memory_requirement A pointer to hold the required memory size of state
 * @param state 0 if just requesting memory requirement, otherwise allocated block of memory
 * @returns True on success; otherwise false.
 */
b8 renderer_system_initialize(u64* renderer_state_memory_requirments, void* state, const char* application_name);
void renderer_system_shutdown(void* state);

void renderer_on_resized(u16 width, u16 height);

b8 renderer_draw_frame(render_packet* packet);