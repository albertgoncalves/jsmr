#ifndef __PRELUDE_H__
#define __PRELUDE_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef FILE File;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int32_t i32;

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

#define ERROR(message)                                             \
    {                                                              \
        fprintf(stderr, "[ERROR] (`%s`) %s\n", __func__, message); \
        exit(EXIT_FAILURE);                                        \
    }

#define NOT_IMPLEMENTED                                              \
    {                                                                \
        fprintf(stderr, "[ERROR] `%s` not implemented\n", __func__); \
        exit(EXIT_FAILURE);                                          \
    }

#endif
