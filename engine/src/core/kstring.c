#include "core/kstring.h"
#include "core/pe_memory.h"

// TODO: stop using string and create own
#include <string.h>

u64 string_length(const char* str) {
    return strlen(str);
}

char* string_duplicate(const char* str) {
    u64 length = string_length(str);
    char* copy = pe_allocate(length + 1, MEMORY_TAG_STRING);
    pe_copy_memory(copy, str, length + 1);
    return copy;
}

// Case-sensitive string comparison. TRUE if the same, otherwise FALSE
b8 strings_equal(const char* str0, const char* str1) {
    return strcmp(str0, str1) == 0;
}