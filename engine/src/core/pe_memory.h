#pragma once

#include "defines.h"

typedef enum memory_tag {
    // For temporary use. Should be assigned one of the below or have a new tag created.
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX_TAGS
} memory_tag;

/**
 * @brief Initializes memory system. Call twice; once with state = 0 to get required memory size,
 * then a second time passing allocated memory to state.
 * 
 * @param memory_requirement A pointer to hold the required memory size state
 * @param state 0 if just requesting memory requirement, otherwise allocated block of memory
 * @returns True on success; otherwise false.
 */
PE_API b8 memory_system_initialize(u64* memory_requirement, void* state);
PE_API void memory_system_shutdown(void* state);


PE_API void* pe_allocate(u64 size, memory_tag tag);

PE_API void pe_free(void* block, u64 size, memory_tag tag);

PE_API void* pe_zero_memory(void* block, u64 size);

PE_API void* pe_copy_memory(void* dst, const void* src, u64 size);

PE_API void* pe_set_memory(void* dst, i32 value, u64 size);


PE_API char* get_memory_usage_str();

PE_API u64 get_memory_alloc_count();