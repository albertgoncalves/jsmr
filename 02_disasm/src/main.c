#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef FILE File;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

#define SIZE_BYTES   1024
#define SIZE_TOKENS  128
#define SIZE_CHARS   512
#define SIZE_UTF8S   48
#define SIZE_ATTRIBS 128

typedef enum {
    MAGIC,
    MINOR_VERSION,
    MAJOR_VERSION,
    CONSTANT_POOL_SIZE,
    CONSTANT,
    ACCESS_FLAGS,
    THIS_CLASS,
    SUPER_CLASS,
    INTERFACES_SIZE,
    // INTERFACE,
    FIELDS_SIZE,
    // FIELD,
    METHODS_SIZE,
    METHOD,
} Tag;

typedef enum {
    CONSTANT_TAG_UTF8 = 1,
    // CONSTANT_TAG_INTEGER = 3,
    // CONSTANT_TAG_FLOAT = 4,
    // CONSTANT_TAG_LONG = 5,
    // CONSTANT_TAG_DOUBLE = 6,
    CONSTANT_TAG_CLASS = 7,
    CONSTANT_TAG_STRING = 8,
    CONSTANT_TAG_FIELD_REF = 9,
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
    u16         size;
    const char* string;
} ConstantUtf8;

typedef struct {
    u16 name_index;
} ConstantClass;

typedef struct {
    u16 string_index;
} ConstantString;

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
        ConstantString      string;
        ConstantClass       class_;
        ConstantRef         ref;
        ConstantNameAndType name_and_type;
    };
    u16         index;
    ConstantTag tag;
} Constant;

typedef enum {
    ACC_PUBLIC = 0x0001,
    ACC_FINAL = 0x0010,
    ACC_SUPER = 0x0020,
    ACC_INTERFACE = 0x0200,
    ACC_ABSTRACT = 0x0400,
    ACC_SYNTHETIC = 0x1000,
    ACC_ANNOTATION = 0x2000,
    ACC_ENUM = 0x4000,
    ACC_MODULE = 0x8000,
} AccessFlag;

typedef enum {
    METHOD_ACC_PUBLIC = 0x0001,
    METHOD_ACC_PRIVATE = 0x0002,
    METHOD_ACC_PROTECTED = 0x0004,
    METHOD_ACC_STATIC = 0x0008,
    METHOD_ACC_FINAL = 0x0010,
    METHOD_ACC_SYNCHRONIZED = 0x0020,
    METHOD_ACC_BRIDGE = 0x0040,
    METHOD_ACC_VARARGS = 0x0080,
    METHOD_ACC_NATIVE = 0x0100,
    METHOD_ACC_ABSTRACT = 0x0400,
    METHOD_ACC_STRICT = 0x0800,
    METHOD_ACC_SYNTHETIC = 0x1000,
} MethodAccessFlag;

typedef enum {
    ATTRIB_CODE,
} AttributeTag;

// typedef struct {
//     u16 pc_start;
//     u16 pc_end;
//     u16 pc_handler;
//     u16 catch_type;
// } ExceptionTable;

typedef struct Attribute Attribute;

typedef struct {
    u16 max_stack;
    u16 max_locals;
    u32 bytes_size;
    u8* bytes;
    // u16             exception_table_size;
    // ExceptionTable* exception_table;
    // u16             attributes_size;
    // Attribute*      attributes;
} CodeAttribute;

typedef enum {
    OP_ALOAD_0 = 42,
    OP_INVOKESPECIAL = 183,
    OP_RETURN = 177,
} OpCode;

struct Attribute {
    u16 name_index;
    u32 size;
    union {
        CodeAttribute code;
    };
    AttributeTag tag;
};

typedef struct {
    u16        access_flags;
    u16        name_index;
    u16        descriptor_index;
    u16        attributes_size;
    Attribute* attributes;
} Method;

typedef struct {
    union {
        u32      u32;
        u16      u16;
        Constant constant;
        Method   method;
    };
    Tag tag;
} Token;

typedef struct {
    usize     file_size;
    u8        bytes[SIZE_BYTES];
    usize     byte_index;
    Token     tokens[SIZE_TOKENS];
    usize     token_index;
    char      chars[SIZE_CHARS];
    char*     utf8s[SIZE_UTF8S];
    usize     char_index;
    Attribute attributes[SIZE_ATTRIBS];
    usize     attribute_index;
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

static Token* alloc_token(Memory* memory) {
    if (SIZE_TOKENS <= memory->token_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new token\n");
        exit(EXIT_FAILURE);
    }
    return &memory->tokens[memory->token_index++];
}

static Attribute* alloc_attribute(Memory* memory) {
    if (SIZE_ATTRIBS <= memory->attribute_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new attribute\n");
        exit(EXIT_FAILURE);
    }
    return &memory->attributes[memory->attribute_index++];
}

static void push_tag_u16(Memory* memory, Tag tag, u16 value) {
    Token* token = alloc_token(memory);
    token->tag = tag;
    token->u16 = value;
}

static void set_tokens(Memory* memory) {
    memory->byte_index = 0;
    memory->token_index = 0;
    memory->char_index = 0;
    memory->attribute_index = 0;
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
    push_tag_u16(memory, MINOR_VERSION, pop_u16(memory));
    push_tag_u16(memory, MAJOR_VERSION, pop_u16(memory));
    {
        u16 constant_pool_size = pop_u16(memory);
        push_tag_u16(memory, CONSTANT_POOL_SIZE, constant_pool_size);
        for (u16 i = 1; i < constant_pool_size; ++i) {
            ConstantTag tag = (ConstantTag)pop_u8(memory);
            Token*      token = alloc_token(memory);
            token->tag = CONSTANT;
            token->constant.index = i;
            token->constant.tag = tag;
            switch (tag) {
            case CONSTANT_TAG_UTF8: {
                u16 utf8_size = pop_u16(memory);
                if (SIZE_CHARS <= (memory->char_index + utf8_size + 1)) {
                    fprintf(stderr, "[ERROR] Unable to allocate string\n");
                    exit(EXIT_FAILURE);
                }
                if (SIZE_UTF8S <= i) {
                    fprintf(stderr,
                            "[ERROR] Unable to allocate UTF8 pointer\n");
                    exit(EXIT_FAILURE);
                }
                token->constant.utf8.size = utf8_size;
                char* utf8 = &memory->chars[memory->char_index];
                token->constant.utf8.string = utf8;
                memory->utf8s[i] = utf8;
                for (u16 j = 0; j < utf8_size; ++j) {
                    memory->chars[memory->char_index++] = (char)pop_u8(memory);
                }
                memory->chars[memory->char_index++] = '\0';
                break;
            }
            case CONSTANT_TAG_CLASS: {
                token->constant.class_.name_index = pop_u16(memory);
                break;
            }
            case CONSTANT_TAG_STRING: {
                token->constant.string.string_index = pop_u16(memory);
                break;
            }
            case CONSTANT_TAG_FIELD_REF:
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
                fprintf(stderr,
                        "[ERROR] `{ ConstantTag tag (%hhu) }` "
                        "unimplemented\n\n",
                        (u8)tag);
                return;
            }
            }
        }
    }
    push_tag_u16(memory, ACCESS_FLAGS, pop_u16(memory));
    push_tag_u16(memory, THIS_CLASS, pop_u16(memory));
    push_tag_u16(memory, SUPER_CLASS, pop_u16(memory));
    {
        u16 interfaces_size = pop_u16(memory);
        push_tag_u16(memory, INTERFACES_SIZE, interfaces_size);
        for (u16 i = 0; i < interfaces_size; ++i) {
            fprintf(stderr, "[ERROR] `{ u16 interface }` unimplemented\n\n");
            break;
        }
    }
    {
        u16 fields_size = pop_u16(memory);
        push_tag_u16(memory, FIELDS_SIZE, fields_size);
        for (u16 i = 0; i < fields_size; ++i) {
            fprintf(stderr, "[ERROR] `{ ? field }` unimplemented\n\n");
            break;
        }
    }
    {
        u16 methods_size = pop_u16(memory);
        push_tag_u16(memory, METHODS_SIZE, methods_size);
        for (u16 i = 0; i < methods_size; ++i) {
            Token* token = alloc_token(memory);
            token->tag = METHOD;
            token->method.access_flags = pop_u16(memory);
            token->method.name_index = pop_u16(memory);
            token->method.descriptor_index = pop_u16(memory);
            u16 attributes_size = pop_u16(memory);
            token->method.attributes_size = attributes_size;
            for (u16 j = 0; j < attributes_size; ++j) {
                Attribute* attribute = alloc_attribute(memory);
                if (j == 0) {
                    token->method.attributes = attribute;
                }
                u16 attribute_name_index = pop_u16(memory);
                u32 attribute_size = pop_u32(memory);
                attribute->name_index = attribute_name_index;
                attribute->size = attribute_size;
                const char* attribute_name =
                    memory->utf8s[attribute_name_index];
                if (!strcmp(attribute_name, "Code")) {
                    attribute->tag = ATTRIB_CODE;
                    attribute->code.max_stack = pop_u16(memory);
                    attribute->code.max_locals = pop_u16(memory);
                    attribute->code.bytes_size = pop_u32(memory);
                    if (memory->file_size <= memory->byte_index) {
                        OUT_OF_BOUNDS;
                    }
                    attribute->code.bytes =
                        &memory->bytes[memory->byte_index++];
                } else {
                    fprintf(stderr,
                            "[DEBUG] %hu\n[DEBUG] %s\n",
                            attribute_name_index,
                            attribute_name);
                    fprintf(stderr,
                            "[ERROR] `{ ? method }` unimplemented\n\n");
                }
            }
            break;
        }
    }
}

static void print_tokens(Memory* memory) {
    Token* tokens = memory->tokens;
    for (usize i = 0; i < memory->token_index; ++i) {
        Token token = tokens[i];
        switch (token.tag) {
        case MAGIC: {
            printf("  0x%-16X(u32 Magic)\n\n", token.u32);
            break;
        }
        case MINOR_VERSION: {
            printf("  %-18hu(u16 MinorVersion)\n", token.u16);
            break;
        }
        case MAJOR_VERSION: {
            printf("  %-18hu(u16 MajorVersion)\n\n", token.u16);
            break;
        }
        case CONSTANT_POOL_SIZE: {
            printf("  %-18hu(u16 ConstantPoolSize)\n\n", token.u16);
            break;
        }
        case CONSTANT: {
            switch (token.constant.tag) {
            case CONSTANT_TAG_UTF8: {
                printf("  %-4hhu%-4hu\"%s\"\n"
                       "                    [%3hu] "
                       "(u8 Constant.Utf8, u16 Length, u8*%hu String)\n",
                       (u8)token.constant.tag,
                       token.constant.utf8.size,
                       token.constant.utf8.string,
                       token.constant.index,
                       token.constant.utf8.size);
                break;
            }
            case CONSTANT_TAG_CLASS: {
                printf("  %-4hhu%-14hu[%3hu] (u8 Constant.Class, "
                       "u16 NameIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.class_.name_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_STRING: {
                printf("  %-4hhu%-14hu[%3hu] (u8 Constant.String, "
                       "u16 StringIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.string.string_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_FIELD_REF: {
                printf("  %-4hhu%-4hu%-10hu[%3hu] "
                       "(u8 Constant.FieldRef, u16 ClassIndex, "
                       "u16 NameAndTypeIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.ref.class_index,
                       token.constant.ref.name_and_type_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_METHOD_REF: {
                printf("  %-4hhu%-4hu%-10hu[%3hu] "
                       "(u8 Constant.MethodRef, u16 ClassIndex, "
                       "u16 NameAndTypeIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.ref.class_index,
                       token.constant.ref.name_and_type_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_NAME_AND_TYPE: {
                printf("  %-4hhu%-4hu%-10hu[%3hu] "
                       "(u8 Constant.NameAndType, u16 NameIndex, "
                       "u16 DescriptorIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.name_and_type.name_index,
                       token.constant.name_and_type.descriptor_index,
                       token.constant.index);
                break;
            }
            }
            break;
        }
        case ACCESS_FLAGS: {
            printf("  0x%-16X(u16 AccessFlags) [", token.u16);
            for (u16 j = 0; j < 16; ++j) {
                switch ((AccessFlag)((1 << j) & token.u16)) {
                case ACC_PUBLIC: {
                    printf(" ACC_PUBLIC");
                    break;
                }
                case ACC_FINAL: {
                    printf(" ACC_FINAL");
                    break;
                }
                case ACC_SUPER: {
                    printf(" ACC_SUPER");
                    break;
                }
                case ACC_INTERFACE: {
                    printf(" ACC_INTERFACE");
                    break;
                }
                case ACC_ABSTRACT: {
                    printf(" ACC_ABSTRACT");
                    break;
                }
                case ACC_SYNTHETIC: {
                    printf(" ACC_SYNTHETIC");
                    break;
                }
                case ACC_ANNOTATION: {
                    printf(" ACC_ANNOTATION");
                    break;
                }
                case ACC_ENUM: {
                    printf(" ACC_ENUM");
                    break;
                }
                case ACC_MODULE: {
                    printf(" ACC_MODULE");
                    break;
                }
                }
            }
            printf(" ]\n\n");
            break;
        }
        case THIS_CLASS: {
            printf("  %-18hu(u16 ThisClass)\n", token.u16);
            break;
        }
        case SUPER_CLASS: {
            printf("  %-18hu(u16 SuperClass)\n", token.u16);
            break;
        }
        case INTERFACES_SIZE: {
            printf("  %-18hu(u16 InterfacesSize)\n", token.u16);
            break;
        }
        case FIELDS_SIZE: {
            printf("  %-18hu(u16 FieldsSize)\n", token.u16);
            break;
        }
        case METHODS_SIZE: {
            printf("  %-18hu(u16 MethodsSize)\n", token.u16);
            break;
        }
        case METHOD: {
            printf("  %-4hu%-4hu%-4hu%-6hu"
                   "(u16 MethodAccessFlags, u16 MethodNameIndex, "
                   "u16 MethodDescriptorIndex, u16 MethodAttributesSize)"
                   "\n",
                   token.method.access_flags,
                   token.method.name_index,
                   token.method.descriptor_index,
                   token.method.attributes_size);
            for (u16 j = 0; j < token.method.attributes_size; ++j) {
                Attribute attribute = token.method.attributes[j];
                printf("  %-4hu%-14u"
                       "(u16 MethodAttributeNameIndex, "
                       "u32 MethodAttributeSize)\n",
                       attribute.name_index,
                       attribute.size);
                switch (attribute.tag) {
                case ATTRIB_CODE: {
                    printf("  %-4hu%-4hu%-10u"
                           "(u16 CodeMaxStack, u16 CodeMaxLocals, "
                           "u32 CodeBytesSize)\n",
                           attribute.code.max_stack,
                           attribute.code.max_locals,
                           attribute.code.bytes_size);
                    printf("  {\n");
                    for (u32 k = 0; k < attribute.code.bytes_size;) {
                        OpCode op_code = attribute.code.bytes[k++];
                        switch (op_code) {
                        case OP_ALOAD_0: {
                            printf("    aload_0\n");
                            break;
                        case OP_INVOKESPECIAL: {
                            u8 byte1 = attribute.code.bytes[k++];
                            u8 byte2 = attribute.code.bytes[k++];
                            u16 index = (u16)((byte1 << 8) | byte2);
                            printf("    invokespecial %hu\n", index);
                            break;
                        }
                        case OP_RETURN: {
                            printf("    return\n");
                            break;
                        }
                        }
                        default: {
                            fprintf(stderr,
                                    "[ERROR] `{ OpCode op_code (%hhu) }` "
                                    "unimplemented\n\n",
                                    (u8)op_code);
                            exit(EXIT_FAILURE);
                        }
                        }
                    }
                    printf("  }\n");
                }
                }
            }
            break;
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
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        fprintf(stderr, "[ERROR] `calloc` failed\n");
        exit(EXIT_FAILURE);
    }
    set_file_to_bytes(memory, args[1]);
    set_tokens(memory);
    fflush(stderr);
    print_tokens(memory);
    fflush(stdout);
    fprintf(stderr,
            "\n[INFO] %zu bytes left!\n",
            memory->file_size - memory->byte_index);
    free(memory);
    return EXIT_SUCCESS;
}
