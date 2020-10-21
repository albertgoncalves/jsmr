#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* NOTE: See
 * `https://medium.com/@davethomas_9528/writing-hello-world-in-java-byte-code-34f75428e0ad`.
 */

typedef FILE File;

typedef uint8_t  u8;
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

static u16 length(const char* x) {
    u16 i = 0;
    while (x[i] != '\0') {
        ++i;
    }
    return i;
}

#define SET_BYTES(file, bytes)                         \
    if (fwrite(bytes, sizeof(*bytes), 1, file) != 1) { \
        exit(EXIT_FAILURE);                            \
    }

#define SET_STRING(file, string, len)                     \
    if (fwrite(string, sizeof(char), len, file) != len) { \
        exit(EXIT_FAILURE);                               \
    }

static void set_constant_pool_class(File*       file,
                                    u16         index,
                                    const char* string) {
    {
        const u8  utf8_id = 1;
        const u16 len = (u16)length(string);
        const u16 swap_len = __builtin_bswap16(len);
        SET_BYTES(file, &utf8_id);
        SET_BYTES(file, &swap_len);
        SET_STRING(file, string, len);
    }
    {
        const u8  class_id = 7;
        const u16 swap_index = __builtin_bswap16(index);
        SET_BYTES(file, &class_id);
        SET_BYTES(file, &swap_index);
    }
}

i32 main(i32 n, const char** args) {
    if (n < 2) {
        exit(EXIT_FAILURE);
    }
    /* NOTE: See `https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html`. */
    const Version version = {
        .magic = __builtin_bswap32(0xCAFEBABE),
        .minor_version = __builtin_bswap16(0),
        .major_version = __builtin_bswap16(58),
    };
    const char* constant_pool[] = {
        "Main",
        "java/lang/Object",
        "java/lang/System",
        "java/io/PrintStream",
        "Hello, world!",
    };
    const u8    m = sizeof(constant_pool) / sizeof(char*);
    const u16   constant_pool_size = __builtin_bswap16((m * 2) + 1);
    const Flags flags = {
        .access_modifiers = __builtin_bswap16(0x0021),
        .class_constant_index = __builtin_bswap16(2),
        .super_constant_index = __builtin_bswap16(4),
    };
    const u16 empty = __builtin_bswap16(0);
    {
        File* file = fopen(args[1], "wb");
        if (file == NULL) {
            exit(EXIT_FAILURE);
        }
        SET_BYTES(file, &version);
        SET_BYTES(file, &constant_pool_size);
        for (u8 i = 0; i < m; ++i) {
            set_constant_pool_class(file,
                                    (u16)((i * 2) + 1),
                                    constant_pool[i]);
        }
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
