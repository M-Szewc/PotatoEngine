#include "pe_math.h"
#include "platform/platform.h"

#include <math.h>
#include <stdlib.h>

static b8 rand_seeded = false;

/**
 * Functions here are in order to prevent from importing the
 * whole <math.h> everywhere.
 */
f32 pe_sin(f32 x){
    return sinf(x);
}

f32 pe_cos(f32 x){
    return cosf(x);
}

f32 pe_tan(f32 x){
    return tanf(x);
}

f32 pe_acos(f32 x){
    return acosf(x);
}

f32 pe_sqrt(f32 x){
    return sqrtf(x);
}

f32 pe_abs(f32 x){
    return fabs(x);
}

i32 pe_random(){
    if (!rand_seeded) {
        srand((u32)platform_get_absolute_time());
        rand_seeded = true;
    }
    return rand();
}

i32 pe_random_in_range(i32 min, i32 max){
    if (!rand_seeded) {
        srand((u32)platform_get_absolute_time());
        rand_seeded = true;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 pe_frandom(){
    return (f32)pe_random() / (f32)RAND_MAX;
}

f32 pe_frandom_in_range(f32 min, f32 max){
    return min + ((f32)pe_random() / ((f32)RAND_MAX / (max - min)));
}
