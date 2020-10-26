#ifndef __MEMORY_H__
#define __MEMORY_H__

#define COUNT_BYTES               1024
#define COUNT_TOKENS              128
#define COUNT_CHARS               512
#define COUNT_UTF8S               48
#define COUNT_ATTRIBS             128
#define COUNT_LINE_NUMBER_ENTRIES 32
#define COUNT_STACK_MAP_ENTRIES   32
#define COUNT_VERIFICATION_TYPES  32

typedef enum {
    MAGIC,
    MINOR_VERSION,
    MAJOR_VERSION,
    CONSTANT_POOL_COUNT,
    CONSTANT,
    ACCESS_FLAGS,
    THIS_CLASS,
    SUPER_CLASS,
    INTERFACES_COUNT,
    // INTERFACE,
    FIELDS_COUNT,
    // FIELD,
    METHODS_COUNT,
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
    ATTRIB_LINE_NUMBER_TABLE,
    ATTRIB_STACK_MAP_TABLE,
} AttributeTag;

// typedef struct {
//     u16 pc_start;
//     u16 pc_end;
//     u16 pc_handler;
//     u16 catch_type;
// } ExceptionTable;

typedef struct Attribute Attribute;

typedef struct {
    Attribute* attributes;
    u8*        bytes;
    u32        bytes_count;
    u16        max_stack;
    u16        max_locals;
    u16        exception_table_count;
    u16        attributes_count;
    // ExceptionTable* exception_table;
} Code;

typedef enum {
    OP_ALOAD_0 = 42,
    OP_INVOKESPECIAL = 183,
    OP_RETURN = 177,
    OP_GETFIELD = 180,
    OP_IFNE = 154,
    OP_ICONST_0 = 3,
    OP_IRETURN = 172,
    OP_ICONST_1 = 4,
    OP_IF_ICMPNE = 160,
    OP_ISTORE_1 = 60,
    OP_ISTORE_2 = 61,
    OP_ISTORE_3 = 62,
    OP_ICONST_2 = 5,
    OP_ISTORE = 54,
    OP_ILOAD = 21,
    OP_IF_ICMPGE = 162,
    OP_ILOAD_2 = 28,
    OP_ILOAD_3 = 29,
    OP_ILOAD_1 = 27,
    OP_IADD = 96,
    OP_IINC = 132,
    OP_GOTO = 167,
} OpCode;

typedef struct {
    u16 pc_start;
    u16 line_number;
} LineNumberEntry;

typedef struct {
    LineNumberEntry* entries;
    u16              count;
} LineNumberTable;

typedef enum {
    VERI_TOP = 0,
    VERI_INTEGER = 1,
    VERI_FLOAT = 2,
    VERI_DOUBLE = 3,
    VERI_LONG = 4,
    VERI_NULL = 5,
    VERI_UNINIT_THIS = 6,
    VERI_OBJECT = 7,
    VERI_UNINIT = 8,
} VerificationTypeTag;

typedef struct {
    union {
        u16 constant_pool_index;
        u16 offset;
    };
    VerificationTypeTag tag;
    u8                  bit_tag;
} VerificationType;

typedef enum {
    STACK_MAP_SAME_FRAME,
    STACK_MAP_SAME_LOCALS_1_STACK_ITEM_FRAME,
    STACK_MAP_SAME_LOCALS_1_STACK_ITEM_FRAME_EXTENDED,
    STACK_MAP_CHOP_FRAME,
    STACK_MAP_SAME_FRAME_EXTENDED,
    STACK_MAP_APPEND_FRAME,
    STACK_MAP_FULL_FRAME,
} StackMapTag;

typedef struct {
    VerificationType* local_items;
    VerificationType* stack_items;
    u16               offset_delta;
    u16               local_item_count;
    u16               stack_item_count;
    StackMapTag       tag;
    u8                bit_tag;
} StackMapEntry;

typedef struct {
    StackMapEntry* entries;
    u16            count;
} StackMapTable;

struct Attribute {
    union {
        Code            code;
        LineNumberTable line_number_table;
        StackMapTable   stack_map_table;
    };
    Attribute*   next_attribute;
    u32          size;
    u16          name_index;
    AttributeTag tag;
};

typedef struct {
    Attribute* attributes;
    u16        access_flags;
    u16        name_index;
    u16        descriptor_index;
    u16        attributes_count;
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
    usize            file_size;
    usize            byte_index;
    u8               bytes[COUNT_BYTES];
    usize            token_index;
    Token            tokens[COUNT_TOKENS];
    usize            char_index;
    char             chars[COUNT_CHARS];
    char*            utf8s_by_index[COUNT_UTF8S];
    usize            attribute_index;
    Attribute        attributes[COUNT_ATTRIBS];
    usize            line_number_entry_index;
    LineNumberEntry  line_number_entries[COUNT_LINE_NUMBER_ENTRIES];
    usize            stack_map_entry_index;
    StackMapEntry    stack_map_entries[COUNT_STACK_MAP_ENTRIES];
    usize            verification_type_index;
    VerificationType verification_types[COUNT_VERIFICATION_TYPES];
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
    if (COUNT_BYTES < file_size) {
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

static u8* pop_u8_ref(Memory* memory) {
    if (memory->file_size <= memory->byte_index) {
        OUT_OF_BOUNDS;
    }
    return &memory->bytes[memory->byte_index++];
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

static u8 pop_u8_at(u8* bytes, u32* index) {
    return bytes[(*index)++];
}

static u16 pop_u16_at(u8* bytes, u32* index) {
    u8 byte1 = bytes[(*index)++];
    u8 byte2 = bytes[(*index)++];
    return (u16)((byte1 << 8) | byte2);
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
    if (COUNT_TOKENS <= memory->token_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new token\n");
        exit(EXIT_FAILURE);
    }
    return &memory->tokens[memory->token_index++];
}

static Attribute* alloc_attribute(Memory* memory) {
    if (COUNT_ATTRIBS <= memory->attribute_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new attribute\n");
        exit(EXIT_FAILURE);
    }
    Attribute* attribute = &memory->attributes[memory->attribute_index++];
    attribute->next_attribute = NULL;
    return attribute;
}

static LineNumberEntry* alloc_line_number_entry(Memory* memory) {
    if (COUNT_LINE_NUMBER_ENTRIES <= memory->line_number_entry_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new line number entry\n");
        exit(EXIT_FAILURE);
    }
    return &memory->line_number_entries[memory->line_number_entry_index++];
}

static StackMapEntry* alloc_stack_map_entry(Memory* memory) {
    if (COUNT_STACK_MAP_ENTRIES <= memory->stack_map_entry_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new stack map entry\n");
        exit(EXIT_FAILURE);
    }
    return &memory->stack_map_entries[memory->stack_map_entry_index++];
}

static VerificationType* alloc_verification_type(Memory* memory) {
    if (COUNT_VERIFICATION_TYPES <= memory->verification_type_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new verification type\n");
        exit(EXIT_FAILURE);
    }
    return &memory->verification_types[memory->verification_type_index++];
}

static void push_tag_u16(Memory* memory, Tag tag, u16 value) {
    Token* token = alloc_token(memory);
    token->tag = tag;
    token->u16 = value;
}

#endif
