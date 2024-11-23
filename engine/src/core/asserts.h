#pragma once

#include "defines.h"

// Disable assertions by commenting out the below line.
#define PE_ASSERTIONS_ENABLED

#ifdef PE_ASSERTIONS_ENABLED
#if _MSC_VER
    #include <intrin.h>
    #define debugBreak() __debugbreak()
#else
    #define debugBreak() __builtint_trap()
#endif

PE_API void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);

#define PE_ASSERT(expr)                                                 \
    {                                                                   \
        if(expr){                                                       \
        } else {                                                        \
            report_assertion_failure(#expr, "", __FILE__, __LINE__);    \
            debugBreak();                                               \
        }                                                               \
    }

#define PE_ASSERT_MSG(expr, message)                                        \
    {                                                                       \
        if(expr){                                                           \
        } else {                                                            \
            report_assertion_failure(#expr, message, __FILE__, __LINE__);   \
            debugBreak();                                                   \
        }                                                                   \
    }

#ifdef _DEBUG
#define PE_ASSERT_DEBUG(expr)                                       \
    {                                                               \
        if(expr){                                                   \
        } else {                                                    \
            report_assertion_failure(#expr, "", __FILE__, __LINE__);\
            debugBreak();                                           \
        }                                                           \
    }

#else
#define PE_ASSERT_DEBUG(expr)
#endif

#else
#define PE_ASSERT(expr)
#define PE_ASSERT_MSG(expr, message)
#define PE_ASSERT_DEBUG(expr)

#endif