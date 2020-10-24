#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef FILE File;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

#define SIZE_BYTES  1024
#define SIZE_TOKENS 128
#define SIZE_CHARS  64

typedef enum {
    MAGIC,
    MINOR_VERSION,
    MAJOR_VERSION,
    CONSTANT_POOL_SIZE,
    CONSTANT,
} Tag;

typedef enum {
    CONSTANT_TAG_UTF8 = 1,
    // CONSTANT_TAG_INTEGER = 3,
    // CONSTANT_TAG_FLOAT = 4,
    // CONSTANT_TAG_LONG = 5,
    // CONSTANT_TAG_DOUBLE = 6,
    CONSTANT_TAG_CLASS = 7,
    // CONSTANT_TAG_STRING = 8,
    // CONSTANT_TAG_FIELD_REF = 9,
    CONSTANT_TAG_METHOD_REF = 10,
    // CONSTANT_TAG_INTERFACE_METHOD_REF = 11,
    CONSTANT_TAG_NAME_AND_TYPE = 12,
    // CONSTANT_TAG_METHOD_HANDLE = 15,
    // CONSTANT_TAG_METHOD_TYPE = 16,
    // CONSTANT_TAG_DYNAMIC = 17,
    // CONSTANT_TAG_INVOKE_DYNAMIC = 18,
    // CONSTANT_TAG_MODULE = 19,
    // CONSTANT_TAG_PACKAGE = 20,
} ConstantTag;

typedef struct {
    u16         length;
    const char* string;
} ConstantUtf8;

typedef struct {
    u16 name_index;
} ConstantClass;

typedef struct {
    u16 class_index;
    u16 name_and_type_index;
} ConstantRef;

typedef struct {
    u16 name_index;
    u16 descriptor_index;
} ConstantNameAndType;

typedef struct {
    union {
        ConstantUtf8        utf8;
        ConstantClass       class_;
        ConstantRef         ref;
        ConstantNameAndType name_and_type;
    };
    u16         index;
    ConstantTag tag;
} Constant;

typedef struct {
    union {
        u32      u32;
        u16      u16;
        Constant constant;
    };
    Tag tag;
} Token;

typedef struct {
    usize file_size;
    u8    bytes[SIZE_BYTES];
    usize byte_index;
    Token tokens[SIZE_TOKENS];
    usize token_index;
    char  chars[SIZE_CHARS];
    usize char_index;
} Memory;

static void set_file_to_bytes(Memory* memory, const char* filename) {
    File* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "[ERROR] Unable to open file\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    usize file_size = (usize)ftell(file);
    rewind(file);
    if (SIZE_BYTES < file_size) {
        fprintf(stderr, "[ERROR] File does not fit into memory\n");
        exit(EXIT_FAILURE);
    }
    if (fread(&memory->bytes, sizeof(u8), file_size, file) != file_size) {
        fprintf(stderr, "[ERROR] `fread` failed\n");
        exit(EXIT_FAILURE);
    }
    memory->file_size = file_size;
    fclose(file);
}

#define OUT_OF_BOUNDS                               \
    {                                               \
        fprintf(stderr, "[ERROR] Out of bounds\n"); \
        exit(EXIT_FAILURE);                         \
    }

static u8 pop_u8(Memory* memory) {
    if (memory->file_size <= memory->byte_index) {
        OUT_OF_BOUNDS;
    }
    return memory->bytes[memory->byte_index++];
}

static u16 pop_u16(Memory* memory) {
    usize next_index = memory->byte_index + 2;
    if (memory->file_size < next_index) {
        OUT_OF_BOUNDS;
    }
    usize i = memory->byte_index;
    u16   bytes = (u16)((memory->bytes[i] << 8) | (memory->bytes[i + 1]));
    memory->byte_index = next_index;
    return bytes;
}

static u32 pop_u32(Memory* memory) {
    usize next_index = memory->byte_index + 4;
    if (memory->file_size < next_index) {
        OUT_OF_BOUNDS;
    }
    usize i = memory->byte_index;
    u32 bytes = (u32)((memory->bytes[i] << 24) | (memory->bytes[i + 1] << 16) |
                      (memory->bytes[i + 2] << 8) | memory->bytes[i + 3]);
    memory->byte_index = next_index;
    return bytes;
}

#undef OUT_OF_BOUNDS

static Token* alloc_token(Memory* memory) {
    if (SIZE_TOKENS < memory->token_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new token\n");
        exit(EXIT_FAILURE);
    }
    return &memory->tokens[memory->token_index++];
}

static void set_tokens(Memory* memory) {
    memory->byte_index = 0;
    memory->token_index = 0;
    memory->char_index = 0;
    {
        u32 magic = pop_u32(memory);
        if (magic != 0xCAFEBABE) {
            fprintf(stderr, "[ERROR] Incorrect magic constant\n");
            exit(EXIT_FAILURE);
        }
        Token* token = alloc_token(memory);
        token->tag = MAGIC;
        token->u32 = magic;
    }
    {
        Token* token = alloc_token(memory);
        token->tag = MINOR_VERSION;
        token->u16 = pop_u16(memory);
    }
    {
        Token* token = alloc_token(memory);
        token->tag = MAJOR_VERSION;
        token->u16 = pop_u16(memory);
    }
    {
        u16 constant_pool_size = pop_u16(memory);
        {
            Token* token = alloc_token(memory);
            token->tag = CONSTANT_POOL_SIZE;
            token->u16 = constant_pool_size;
        }
        for (u16 i = 1; i < constant_pool_size; ++i) {
            Token* token = alloc_token(memory);
            token->tag = CONSTANT;
            token->constant.index = i;
            ConstantTag tag = (ConstantTag)pop_u8(memory);
            token->constant.tag = tag;
            switch (tag) {
            case CONSTANT_TAG_UTF8: {
                u16 length = pop_u16(memory);
                if (SIZE_CHARS < (memory->char_index + length + 1)) {
                    fprintf(stderr, "[ERROR] Unable to allocate string\n");
                    exit(EXIT_FAILURE);
                }
                token->constant.utf8.length = length;
                token->constant.utf8.string =
                    &memory->chars[memory->char_index];
                for (u16 j = 0; j < length; ++j) {
                    memory->chars[memory->char_index++] = (char)pop_u8(memory);
                }
                memory->chars[memory->char_index++] = '\0';
                break;
            }
            case CONSTANT_TAG_CLASS: {
                token->constant.class_.name_index = pop_u16(memory);
                break;
            }
            case CONSTANT_TAG_METHOD_REF: {
                token->constant.ref.class_index = pop_u16(memory);
                token->constant.ref.name_and_type_index = pop_u16(memory);
                break;
            }
            case CONSTANT_TAG_NAME_AND_TYPE: {
                token->constant.name_and_type.name_index = pop_u16(memory);
                token->constant.name_and_type.descriptor_index =
                    pop_u16(memory);
                break;
            }
            default: {
                printf("! %hhu !\n\n", (u8)tag);
                return;
            }
            }
        }
    }
}

static void print_tokens(Memory* memory) {
    Token* tokens = memory->tokens;
    for (usize i = 0; i < memory->token_index; ++i) {
        Token token = tokens[i];
        switch (token.tag) {
        case MAGIC: {
            printf("  0x%-12X(u32 Magic)\n\n", token.u32);
            break;
        }
        case MINOR_VERSION: {
            printf("  %-14hu(u16 MinorVersion)\n", token.u16);
            break;
        }
        case MAJOR_VERSION: {
            printf("  %-14hu(u16 MajorVersion)\n\n", token.u16);
            break;
        }
        case CONSTANT_POOL_SIZE: {
            printf("  %-14hu(u16 ConstantPoolSize)\n", token.u16);
            break;
        }
        case CONSTANT: {
            switch (token.constant.tag) {
            case CONSTANT_TAG_UTF8: {
                printf("  %-4hhu%-4hu%s\n"
                       "                "
                       "(u8 Constant.Utf8, u16 Length, u8*%hu String)\n",
                       (u8)token.constant.tag,
                       token.constant.utf8.length,
                       token.constant.utf8.string,
                       token.constant.utf8.length);
                break;
            }
            case CONSTANT_TAG_CLASS: {
                printf("  %-4hhu%-10hu(u8 Constant.Class, u16 NameIndex)\n",
                       (u8)CONSTANT_TAG_METHOD_REF,
                       token.constant.class_.name_index);
                break;
            }
            case CONSTANT_TAG_METHOD_REF: {
                printf("  %-4hhu%-4hu%-6hu"
                       "(u8 Constant.MethodRef, u16 ClassIndex, "
                       "u16 NameAndTypeIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.ref.class_index,
                       token.constant.ref.name_and_type_index);
                break;
            }
            case CONSTANT_TAG_NAME_AND_TYPE: {
                printf("  %-4hhu%-4hu%-6hu"
                       "(u8 Constant.NameAndType, u16 NameIndex, "
                       "u16 DescriptorIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.name_and_type.name_index,
                       token.constant.name_and_type.descriptor_index);
                break;
            }
            }
        }
        }
    }
}

i32 main(i32 n, const char** args) {
    printf("sizeof(u8)          : %zu\n"
           "sizeof(Tag)         : %zu\n"
           "sizeof(ConstantTag) : %zu\n"
           "sizeof(Constant)    : %zu\n"
           "sizeof(Token)       : %zu\n"
           "sizeof(Memory)      : %zu\n"
           "\n",
           sizeof(u8),
           sizeof(Tag),
           sizeof(ConstantTag),
           sizeof(Constant),
           sizeof(Token),
           sizeof(Memory));
    if (n < 2) {
        fprintf(stderr, "[ERROR] No file provided\n");
        exit(EXIT_FAILURE);
    }
    Memory* memory = malloc(sizeof(Memory));
    if (memory == NULL) {
        fprintf(stderr, "[ERROR] `malloc` failed\n");
        exit(EXIT_FAILURE);
    }
    set_file_to_bytes(memory, args[1]);
    set_tokens(memory);
    print_tokens(memory);
    fprintf(stderr,
            "\n\n[INFO] %zu bytes left!\n",
            memory->file_size - memory->byte_index);
    free(memory);
    return EXIT_SUCCESS;
}
