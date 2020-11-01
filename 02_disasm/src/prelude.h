#ifndef __PRELUDE_H__
#define __PRELUDE_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef FILE File;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

static u16 get_len(const char* x) {
    u16 i = 0;
    while (x[i] != '\0') {
        ++i;
    }
    return i;
}

static Bool get_eq(const char* a, const char* b) {
    u16 n = get_len(a);
    if (n != get_len(b)) {
        return FALSE;
    }
    for (u16 i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

#endif
