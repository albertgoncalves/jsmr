#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* NOTE: See `https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html`. */

typedef FILE File;

typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t  i32;

// ClassFile {
//     u4               magic;
//     u2               minor_version;
//     u2               major_version;
//
//     u2               constant_pool_count;
//     cp_info          constant_pool[constant_pool_count-1];
//
//     u2               access_flags;
//     u2               this_class_index;
//     u2               super_class_index;
//
//     u2               interfaces_count;
//     u2               interfaces[interfaces_count];
//
//     u2               fields_count;
//     field_info       fields[fields_count];
//
//     u2               methods_count;
//     method_info      methods[methods_count];
//
//     u2               attributes_count;
//     attribute_info   attributes[attributes_count];
// }

typedef struct {
    u32 magic;
    u16 minor_version;
    u16 major_version;
} Version;

typedef struct {
    u16 access_modifiers;
    u16 class_constant_index;
    u16 super_constant_index;
} Flags;

#define SET_BYTES(file, bytes)                                      \
    if (fwrite(bytes, 1, sizeof(*bytes), file) != sizeof(*bytes)) { \
        exit(EXIT_FAILURE);                                         \
    }

i32 main(i32 n, const char** args) {
    if (n < 2) {
        exit(EXIT_FAILURE);
    }
    const Version version = {
        .magic = __builtin_bswap32(0xCAFEBABE),
        .minor_version = __builtin_bswap16(0),
        .major_version = __builtin_bswap16(58),
    };
    const Flags flags = {
        .access_modifiers = __builtin_bswap16(0x0021),
        .class_constant_index = __builtin_bswap16(0),
        .super_constant_index = __builtin_bswap16(0),
    };
    const u16 empty = __builtin_bswap16(0);
    {
        File* file = fopen(args[1], "wb");
        if (file == NULL) {
            exit(EXIT_FAILURE);
        }
        SET_BYTES(file, &version);
        SET_BYTES(file, &empty);
        SET_BYTES(file, &flags);
        SET_BYTES(file, &empty);
        SET_BYTES(file, &empty);
        SET_BYTES(file, &empty);
        SET_BYTES(file, &empty);
        fclose(file);
    }
    printf("Done!\n");
    return EXIT_SUCCESS;
}
