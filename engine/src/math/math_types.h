#pragma once

#include "defines.h"

typedef union vec2_u {
    // An array of x, y
    f32 elements[2];
    struct {
        union {
            // The first element
            f32 x, r, s, u;
        };
        union {
            // the second element
            f32 y, g, t, v;
        };
    };
} vec2;

typedef union vec3_u {
    // An array of x, y, z
    f32 elements[3];
    struct {
        union {
            // The first element
            f32 x, r, s, u;
        };
        union {
            // the second element
            f32 y, g, t, v;
        };
        union {
            // the third element
            f32 z, b, p, w;
        };
    };
} vec3;

typedef union vec4_u {
    // An array of x, y, z, w
    f32 elements[4];
    struct {
        union {
            // The first element
            f32 x, r, s;
        };
        union {
            // the second element
            f32 y, g, t;
        };
        union {
            // the third element
            f32 z, b, p;
        };
        union {
            // the fourth element
            f32 w, a, q;
        };
    };
} vec4;

typedef vec4 quat;

typedef union mat_4 {
    f32 data[16];
} mat4;