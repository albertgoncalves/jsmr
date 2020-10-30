#ifndef __PROGRAM_H__
#define __PROGRAM_H__

typedef enum {
    CONST_CLASS,
    CONST_UTF8,
} ConstantTag;

typedef struct {
    union {
        const char* string;
        u16         name_index;
    };
    ConstantTag tag;
} Constant;

typedef struct {
    u16             major_version;
    u16             minor_version;
    u16             constant_count;
    const Constant* constants;
    u16             access_flags;
} Program;

#endif
