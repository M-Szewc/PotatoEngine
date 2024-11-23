#include "pe_memory.h"

#include "core/logger.h"
#include "platform/platform.h"

// TODO: Custom string lib
#include <string.h>
#include <stdio.h>

struct memory_stats {
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "ARRAY      ",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      "
};

static struct memory_stats stats;

void initialize_memory() {
    platform_zero_memory(&stats, sizeof(stats));
}

void shutdown_memory() {

}

void* pe_allocate(u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        PE_WARN("pe_allocate called using MEMORY_TAG_UNKNOWN. Re-clsss this allocatoin.");
    }

    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;

    // TODO: Memory alignment
    void* block = platform_allocate(size, FALSE);
    platform_zero_memory(block, size);

    return block;
}

void pe_free(void* block, u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        PE_WARN("pe_free called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    stats.total_allocated -= size;
    stats.tagged_allocations[tag] -= size;

    // TODO: Memory alignment
    platform_free(block, FALSE);
}

void* pe_zero_memory(void* block, u64 size) {
    return platform_zero_memory(block, size);
}

void* pe_copy_memory(void* dst, const void* src, u64 size) {
    return platform_copy_memory(dst, src, size);
}

void* pe_set_memory(void* dst, i32 value, u64 size) {
    return platform_set_memory(dst, value, size);
}

char* get_memory_usage_str() {
    const u64 gib = (1<<30);
    const u64 mib = (1<<20);
    const u64 kib = (1<<10);

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "xib";
        f32 amount = 1.0f;
        if (stats.tagged_allocations[i] >= gib) {
            unit[0] = 'G';
            amount = stats.tagged_allocations[i] / (f32)gib;
        } else if (stats.tagged_allocations[i] >= mib) {
            unit[0] = 'M';
            amount = stats.tagged_allocations[i] / (f32)mib;
        } else if (stats.tagged_allocations[i] >= kib) {
            unit[0] = 'K';
            amount = stats.tagged_allocations[i] / (f32)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (f32)stats.tagged_allocations[i];
        }

        offset += snprintf(buffer + offset, 8000 - offset, " %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
    }

    // TODO: change string duplication
    char* out_string = _strdup(buffer);
    return out_string;
}