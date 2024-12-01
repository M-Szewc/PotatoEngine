#pragma once

#include "defines.h"

// Returns the length of the given string
PE_API u64 string_length(const char* str);

PE_API char* string_duplicate(const char* str);

// Case-sensitive string comparison. TRUE if the same, otherwise FALSE
PE_API b8 strings_equal(const char* str0, const char* str1);
