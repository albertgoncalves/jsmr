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

typedef struct {
    u16 size;
    u16 access_modifiers;
    u16 name_index;
    u16 type_index;
    u16 attribute_size;
    u16 code_index;
    u32 code_attribute_size;
    u16 max_stack_size;
    u16 max_local_var_size;
    u32 code_size;
} Method;

static const u16 EMPTY = 0;

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

static void set_constant_pool_utf8(File* file, const char* string) {
    const u8  utf8_op_code = 1;
    const u16 len = (u16)length(string);
    const u16 swap_len = __builtin_bswap16(len);
    SET_BYTES(file, &utf8_op_code);
    SET_BYTES(file, &swap_len);
    if (fwrite(string, sizeof(char), len, file) != len) {
        exit(EXIT_FAILURE);
    }
}

static void set_constant_pool_class(File*       file,
                                    u16         index,
                                    const char* string) {
    set_constant_pool_utf8(file, string);
    const u8  class_op_code = 7;
    const u16 swap_index = __builtin_bswap16(index);
    SET_BYTES(file, &class_op_code);
    SET_BYTES(file, &swap_index);
}

static void set_constant_pool_string(File*       file,
                                     u16         index,
                                     const char* string) {
    set_constant_pool_utf8(file, string);
    const u8  class_op_code = 8;
    const u16 swap_index = __builtin_bswap16(index);
    SET_BYTES(file, &class_op_code);
    SET_BYTES(file, &swap_index);
}

static void set_constant_pool_indices(File* file,
                                      u8    op_code,
                                      u16   index_a,
                                      u16   index_b) {
    const u16 swap_index_a = __builtin_bswap16(index_a);
    const u16 swap_index_b = __builtin_bswap16(index_b);
    SET_BYTES(file, &op_code);
    SET_BYTES(file, &swap_index_a);
    SET_BYTES(file, &swap_index_b);
}

static void set_instr_getstatic(File* file, u16 index) {
    const u8  op_code = 178;
    const u16 swap_index = __builtin_bswap16(index);
    SET_BYTES(file, &op_code);
    SET_BYTES(file, &swap_index);
}

static void set_instr_ldc(File* file, u8 index) {
    const u8 op_code = 18;
    SET_BYTES(file, &op_code);
    SET_BYTES(file, &index);
}

static void set_instr_invokevirtual(File* file, u16 index) {
    const u8  op_code = 182;
    const u16 swap_index = __builtin_bswap16(index);
    SET_BYTES(file, &op_code);
    SET_BYTES(file, &swap_index);
}

static void set_instr_retvoid(File* file) {
    const u8 op_code = 177;
    SET_BYTES(file, &op_code);
}

i32 main(i32 n, const char** args) {
    if (n < 2) {
        exit(EXIT_FAILURE);
    }
    File* file = fopen(args[1], "wb");
    if (file == NULL) {
        exit(EXIT_FAILURE);
    }
    /* NOTE: See `https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html`. */
    {
        const Version version = {
            .magic = __builtin_bswap32(0xCAFEBABE),
            .minor_version = 0,
            .major_version = __builtin_bswap16(58),
        };
        SET_BYTES(file, &version);
    }
    {
        const u16 constant_pool_size = __builtin_bswap16(22);
        SET_BYTES(file, &constant_pool_size);
        set_constant_pool_class(file, 1, "Main");
        set_constant_pool_class(file, 3, "java/lang/Object");
        set_constant_pool_class(file, 5, "java/lang/System");
        set_constant_pool_class(file, 7, "java/io/PrintStream");
        set_constant_pool_string(file, 9, "Hello, world!");
        set_constant_pool_utf8(file, "out");
        set_constant_pool_utf8(file, "Ljava/io/PrintStream;");
        set_constant_pool_indices(file, 12, 11, 12);
        set_constant_pool_indices(file, 9, 6, 13);
        set_constant_pool_utf8(file, "println");
        set_constant_pool_utf8(file, "(Ljava/lang/String;)V");
        set_constant_pool_indices(file, 12, 15, 16);
        set_constant_pool_indices(file, 10, 8, 17);
        set_constant_pool_utf8(file, "main");
        set_constant_pool_utf8(file, "([Ljava/lang/String;)V");
        set_constant_pool_utf8(file, "Code");
    }
    {
        const Flags flags = {
            .access_modifiers = __builtin_bswap16(33),
            .class_constant_index = __builtin_bswap16(2),
            .super_constant_index = __builtin_bswap16(4),
        };
        SET_BYTES(file, &flags);
    }
    {
        /* NOTE: Nothing inscribed into `interfaces` section. */
        SET_BYTES(file, &EMPTY);
    }
    {
        /* NOTE: Nothing inscribed into `fields` section. */
        SET_BYTES(file, &EMPTY);
    }

    {
        const Method method = {
            .size = __builtin_bswap16(1),
            .access_modifiers = __builtin_bswap16(9),
            .name_index = __builtin_bswap16(19),
            .type_index = __builtin_bswap16(20),
            .attribute_size = __builtin_bswap16(1),
            .code_index = __builtin_bswap16(21),
            .code_attribute_size = __builtin_bswap32(21),
            .max_stack_size = __builtin_bswap16(2),
            .max_local_var_size = __builtin_bswap16(1),
            .code_size = __builtin_bswap32(9),
        };
        SET_BYTES(file, &method.size);
        SET_BYTES(file, &method.access_modifiers);
        SET_BYTES(file, &method.name_index);
        SET_BYTES(file, &method.type_index);
        SET_BYTES(file, &method.attribute_size);
        SET_BYTES(file, &method.code_index);
        SET_BYTES(file, &method.code_attribute_size);
        SET_BYTES(file, &method.max_stack_size);
        SET_BYTES(file, &method.max_local_var_size);
        SET_BYTES(file, &method.code_size);
        {
            set_instr_getstatic(file, 14);
            set_instr_ldc(file, 10);
            set_instr_invokevirtual(file, 18);
            set_instr_retvoid(file);
        }
        SET_BYTES(file, &EMPTY);
        SET_BYTES(file, &EMPTY);
    }
    {
        /* NOTE: Nothing inscribed into `attributes` section. */
        SET_BYTES(file, &EMPTY);
    }
    fclose(file);
    printf("Done!\n");
    return EXIT_SUCCESS;
}
