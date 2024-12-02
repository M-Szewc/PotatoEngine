#pragma once

// Unsigned int types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// signed int types
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// used instead of UINT32_MAX, UINT64_MAX, because VScode intellisense
// mistakes windows macros for errors
#define U32MAX (unsigned int)0xffffffff
#define U64MAX (unsigned long long)0xffffffffffffffff

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef int b32;
typedef _Bool b8;

// Properly define static assertions.
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// Ensure all types are of the correct size.
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

#define true 1
#define false 0

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// Windows OS
    #define PE_PLATFORM_WINDOWS 1
    #ifndef _WIN64
        #error "64-bit is required on Windows!"
    #endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
    #define PE_PLATFORM_LINUX 1
    #if defined(__ANDROID__)
        #define PE_PLATFORM_ANDROID 1
    #endif
#elif defined(__unix__)
// Catch anything not caught by the above.
    #define PE_PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
    #define PE_PLATFORM_POSIX 1
#elif __APPLE__
// Apple platforms
    #define PE_PLATFORM_APPLE 1
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
    // IOS Simulator
        #define PE_PLATFORM_IOS 1
        #define PE_PLATFORM_IOS_SIMULATOR 1
    #elif TARGET_OS_IPHONE
    // IOS device
        #define PE_PLATFORM_IOS 1
    #elif TARGET_OS_MAC
    // Other kinds of Mac OS
    #else
        #error "Unknown Apple platform"
    #endif
#else
    #error "Unknown platform"
#endif

#ifdef PE_EXPORT
// Exports
    #ifdef _MSC_VER
        #define PE_API __declspec(dllexport)
    #else
        #define PE_API __attribute__((visibility("default")))
    #endif
#else
// Imports
    #ifdef _MSC_VER
        #define PE_API __declspec(dllimport)
    #else
        #define PE_API
    #endif
#endif

#define PE_CLAMP(value, min, max) ((value <= min) ? min: ((value >= max) ? max : value))


// Inlining
#ifdef _MSC_VER
#define PE_INLINE __forceinline
#define PE_NOINLINE __declspec(noinline)
#else
#define PE_INLINE static inline
#define PE_NOINLINE
#endif 