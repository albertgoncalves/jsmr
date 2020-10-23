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

typedef int32_t i32;

typedef enum {
    ACCESS_PUBLIC = 0x0001,
    ACCESS_PRIVATE = 0x0002,
    ACCESS_PROTECTED = 0x0004,
    ACCESS_STATIC = 0x0008,
    ACCESS_FINAL = 0x0010,
    ACCESS_SUPER = 0x0020,
    ACCESS_VOLATILE = 0x0040,
    ACCESS_TRANSIENT = 0x0080,
    ACCESS_INTERFACE = 0x0200,
    ACCESS_ABSTRACT = 0x0400,
    ACCESS_SYNTHETIC = 0x1000,
    ACCESS_ANNOTATION = 0x2000,
    ACCESS_ENUM = 0x4000,
    ACCESS_MODULE = 0x8000,
} Access;

typedef enum {
    CONSTANT_UTF8 = 1,
    CONSTANT_INTEGER = 3,
    CONSTANT_FLOAT = 4,
    CONSTANT_LONG = 5,
    CONSTANT_DOUBLE = 6,
    CONSTANT_CLASS = 7,
    CONSTANT_STRING = 8,
    CONSTANT_FIELD_REF = 9,
    CONSTANT_METHOD_REF = 10,
    CONSTANT_INTERFACE_METHODREF = 11,
    CONSTANT_NAME_AND_TYPE = 12,
    CONSTANT_METHOD_HANDLE = 15,
    CONSTANT_METHOD_TYPE = 16,
    CONSTANT_DYNAMIC = 17,
    CONSTANT_INVOKE_DYNAMIC = 18,
    CONSTANT_MODULE = 19,
    CONSTANT_PACKAGE = 20,
} Constant;

typedef enum {
    INSTR_LDC = 0x12,
    INSTR_RETVOID = 0xb1,
    INSTR_GETSTATIC = 0xb2,
    INSTR_INVOKEVIRTUAL = 0xb6,
} Instr;

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

static void set_u8(File* file, u8 bytes) {
    SET_BYTES(file, &bytes);
}

static void set_u16(File* file, u16 bytes) {
    /* NOTE: See `https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html`. */
    const u16 swap_bytes = __builtin_bswap16(bytes);
    SET_BYTES(file, &swap_bytes);
}

static void set_constant_pool_utf8(File* file, const char* string) {
    const u16 len = (u16)length(string);
    set_u8(file, CONSTANT_UTF8);
    set_u16(file, len);
    if (fwrite(string, sizeof(char), len, file) != len) {
        exit(EXIT_FAILURE);
    }
}

static void set_constant_pool_class(File*       file,
                                    u16         index,
                                    const char* string) {
    set_constant_pool_utf8(file, string);
    set_u8(file, CONSTANT_CLASS);
    set_u16(file, index);
}

static void set_constant_pool_string(File*       file,
                                     u16         index,
                                     const char* string) {
    set_constant_pool_utf8(file, string);
    set_u8(file, CONSTANT_STRING);
    set_u16(file, index);
}

static void set_constant_pool_indices(File*    file,
                                      Constant op_code,
                                      u16      index_a,
                                      u16      index_b) {
    set_u8(file, op_code);
    set_u16(file, index_a);
    set_u16(file, index_b);
}

static void set_instr_empty(File* file, Instr instr) {
    set_u8(file, instr);
}

static void set_instr_u8(File* file, Instr instr, u8 bytes) {
    set_u8(file, instr);
    set_u8(file, bytes);
}

static void set_instr_u16(File* file, Instr instr, u16 bytes) {
    set_u8(file, instr);
    set_u16(file, bytes);
}

i32 main(i32 n, const char** args) {
    if (n < 2) {
        exit(EXIT_FAILURE);
    }
    File* file = fopen(args[1], "wb");
    if (file == NULL) {
        exit(EXIT_FAILURE);
    }
    {
        const Version version = {
            .magic = __builtin_bswap32(0xCAFEBABE),
            .minor_version = 0,
            .major_version = __builtin_bswap16(58),
        };
        SET_BYTES(file, &version);
    }
    {
        const u16 constant_pool_size = 22;
        set_u16(file, constant_pool_size);
        set_constant_pool_class(file, 1, "Main");
        set_constant_pool_class(file, 3, "java/lang/Object");
        set_constant_pool_class(file, 5, "java/lang/System");
        set_constant_pool_class(file, 7, "java/io/PrintStream");
        set_constant_pool_string(file, 9, "Hello, world!");
        set_constant_pool_utf8(file, "out");
        set_constant_pool_utf8(file, "Ljava/io/PrintStream;");
        set_constant_pool_indices(file, CONSTANT_NAME_AND_TYPE, 11, 12);
        set_constant_pool_indices(file, CONSTANT_FIELD_REF, 6, 13);
        set_constant_pool_utf8(file, "println");
        set_constant_pool_utf8(file, "(Ljava/lang/String;)V");
        set_constant_pool_indices(file, CONSTANT_NAME_AND_TYPE, 15, 16);
        set_constant_pool_indices(file, CONSTANT_METHOD_REF, 8, 17);
        set_constant_pool_utf8(file, "main");
        set_constant_pool_utf8(file, "([Ljava/lang/String;)V");
        set_constant_pool_utf8(file, "Code");
    }
    {
        const Flags flags = {
            .access_modifiers =
                __builtin_bswap16(ACCESS_PUBLIC + ACCESS_SUPER),
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
            .access_modifiers =
                __builtin_bswap16(ACCESS_PUBLIC + ACCESS_STATIC),
            .name_index = __builtin_bswap16(19),
            .type_index = __builtin_bswap16(20),
            .attribute_size = __builtin_bswap16(1),
            .code_index = __builtin_bswap16(21),
            .code_attribute_size = __builtin_bswap32(21),
            .max_stack_size = __builtin_bswap16(2),
            .max_local_var_size = __builtin_bswap16(1),
            .code_size = __builtin_bswap32(9),
        };
        SET_BYTES(file, &method);
        set_instr_u16(file, INSTR_GETSTATIC, 14);
        set_instr_u8(file, INSTR_LDC, 10);
        set_instr_u16(file, INSTR_INVOKEVIRTUAL, 18);
        set_instr_empty(file, INSTR_RETVOID);
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
