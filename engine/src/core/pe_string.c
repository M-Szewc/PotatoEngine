#include "core/pe_string.h"
#include "core/pe_memory.h"

// TODO: stop using string and create own
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

u64 string_length(const char* str) {
    return strlen(str);
}

char* string_duplicate(const char* str) {
    u64 length = string_length(str);
    char* copy = pe_allocate(length + 1, MEMORY_TAG_STRING);
    pe_copy_memory(copy, str, length + 1);
    return copy;
}

// Case-sensitive string comparison. true if the same, otherwise false
b8 strings_equal(const char* str0, const char* str1) {
    return strcmp(str0, str1) == 0;
}

i32 string_format(char* dest, const char* format, ...) {
    if (dest) {
        va_list arg_ptr;
        va_start(arg_ptr, format);
        i32 written = string_format_v(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

i32 string_format_v(char* dest, const char* format, void* va_listp) {
    if (dest) {
        // Big, but can fir on the stack
        char buffer[32000];
        i32 written = vsnprintf(buffer, 32000, format, va_listp);
        buffer[written] = 0;
        pe_copy_memory(dest, buffer,  written + 1);

        return written;
    }
    return -1;
}