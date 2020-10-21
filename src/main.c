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

static void set_constant_pool_string(File* file, const char* string) {
    const u8  utf8_id = 1;
    const u16 len = (u16)length(string);
    const u16 swap_len = __builtin_bswap16(len);
    SET_BYTES(file, &utf8_id);
    SET_BYTES(file, &swap_len);
    if (fwrite(string, sizeof(char), len, file) != len) {
        exit(EXIT_FAILURE);
    }
}

static void set_constant_pool_class(File*       file,
                                    u16         index,
                                    const char* string) {
    set_constant_pool_string(file, string);
    const u8  class_id = 7;
    const u16 swap_index = __builtin_bswap16(index);
    SET_BYTES(file, &class_id);
    SET_BYTES(file, &swap_index);
}

static void set_constant_pool_indices(File* file,
                                      u8    id,
                                      u16   index_a,
                                      u16   index_b) {
    const u16 swap_index_a = __builtin_bswap16(index_a);
    const u16 swap_index_b = __builtin_bswap16(index_b);
    SET_BYTES(file, &id);
    SET_BYTES(file, &swap_index_a);
    SET_BYTES(file, &swap_index_b);
}

i32 main(i32 n, const char** args) {
    if (n < 2) {
        exit(EXIT_FAILURE);
    }
    /* NOTE: See `https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html`. */
    const Version version = {
        .magic = __builtin_bswap32(0xCAFEBABE),
        .minor_version = 0,
        .major_version = __builtin_bswap16(58),
    };
    const u16   constant_pool_size = __builtin_bswap16(22);
    const Flags flags = {
        .access_modifiers = __builtin_bswap16(33),
        .class_constant_index = __builtin_bswap16(2),
        .super_constant_index = __builtin_bswap16(4),
    };
    const u16 method_size = __builtin_bswap16(1);
    const u16 empty = 0;
    {
        File* file = fopen(args[1], "wb");
        if (file == NULL) {
            exit(EXIT_FAILURE);
        }
        SET_BYTES(file, &version);
        SET_BYTES(file, &constant_pool_size);
        set_constant_pool_class(file, 1, "Main");
        set_constant_pool_class(file, 3, "java/lang/Object");
        set_constant_pool_class(file, 5, "java/lang/System");
        set_constant_pool_class(file, 7, "java/io/PrintStream");
        set_constant_pool_class(file, 9, "Hello, world!");
        {
            set_constant_pool_string(file, "out");
            set_constant_pool_string(file, "Ljava/io/PrintStream;");
            set_constant_pool_indices(file, 12, 11, 12);
            set_constant_pool_indices(file, 9, 6, 13);
        }
        {
            set_constant_pool_string(file, "println");
            set_constant_pool_string(file, "(Ljava/lang/String;)V");
            set_constant_pool_indices(file, 12, 15, 16);
            set_constant_pool_indices(file, 10, 8, 17);
        }
        set_constant_pool_string(file, "main");
        set_constant_pool_string(file, "([Ljava/lang/String;)V");
        set_constant_pool_string(file, "Code");
        SET_BYTES(file, &flags);
        SET_BYTES(file, &empty);
        SET_BYTES(file, &empty);
        SET_BYTES(file, &method_size);
        {
            u16 method_access_modifiers = __builtin_bswap16(9);
            u16 index_name = __builtin_bswap16(19);
            u16 index_type = __builtin_bswap16(20);
            SET_BYTES(file, &method_access_modifiers);
            SET_BYTES(file, &index_name);
            SET_BYTES(file, &index_type);
        }
        SET_BYTES(file, &empty);
        SET_BYTES(file, &empty);
        fclose(file);
    }
    printf("Done!\n");
    return EXIT_SUCCESS;
}
