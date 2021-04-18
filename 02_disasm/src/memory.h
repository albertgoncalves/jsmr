#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "prelude.h"

#define COUNT_BYTES               1024
#define COUNT_TOKENS              64
#define COUNT_CHARS               512
#define COUNT_UTF8S               64
#define COUNT_ATTRIBS             64
#define COUNT_LINE_NUMBER_ENTRIES 32
#define COUNT_STACK_MAP_ENTRIES   8
#define COUNT_VERIFICATION_TYPES  8
#define COUNT_NEST_MEMBER_CLASSES 8
#define COUNT_INNER_CLASS_ENTRIES 8

typedef enum {
    MAGIC,
    MINOR_VERSION,
    MAJOR_VERSION,
    CONSTANT_POOL_COUNT,
    CONSTANT,
    ACCESS_FLAGS,
    THIS_CLASS,
    SUPER_CLASS,
    INTERFACE_COUNT,
    // INTERFACE,
    FIELD_COUNT,
    // FIELD,
    METHOD_COUNT,
    METHOD,
    ATTRIBUTE_COUNT,
    ATTRIBUTE,
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
    const char* string;
    u16         size;
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
    ATTRIB_SOURCE_FILE,
    ATTRIB_NEST_MEMBER,
    ATTRIB_INNER_CLASSES,
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
    const u8*  bytes;
    u32        byte_count;
    u16        max_stack;
    u16        max_local;
    u16        exception_table_count;
    u16        attribute_count;
    // ExceptionTable* exception_table;
} Code;

typedef enum {
    OP_ALOAD_0 = 42,
    OP_INVOKESPECIAL = 183,
    OP_RETURN = 177,
    OP_GETFIELD = 180,
    OP_IFNE = 154,
    OP_IRETURN = 172,
    OP_IF_ICMPNE = 160,
    OP_ISTORE_1 = 60,
    OP_ISTORE_2 = 61,
    OP_ISTORE_3 = 62,
    OP_ICONST_0 = 3,
    OP_ICONST_1 = 4,
    OP_ICONST_2 = 5,
    OP_ICONST_3 = 6,
    OP_ISTORE = 54,
    OP_ILOAD = 21,
    OP_IF_ICMPGE = 162,
    OP_ILOAD_2 = 28,
    OP_ILOAD_3 = 29,
    OP_ILOAD_1 = 27,
    OP_IADD = 96,
    OP_ISUB = 100,
    OP_IINC = 132,
    OP_GOTO = 167,
    OP_BIPUSH = 16,
    OP_NEW = 187,
    OP_DUP = 89,
    OP_ASTORE_2 = 77,
    OP_ALOAD_2 = 44,
    OP_PUTFIELD = 181,
    OP_GETSTATIC = 178,
    OP_INVOKESTATIC = 184,
    OP_INVOKEVIRTUAL = 182,
    OP_LDC = 18,
    OP_ILOAD_0 = 26,
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

typedef struct {
    u16* classes;
    u16  count;
} NestMember;

typedef struct {
    u16 inner_class_info_index;
    u16 outer_class_info_index;
    u16 inner_name_index;
    u16 inner_class_access_flags;
} InnerClassEntry;

typedef struct {
    InnerClassEntry* entries;
    u16              count;
} InnerClasses;

struct Attribute {
    union {
        u16             u16;
        Code            code;
        LineNumberTable line_number_table;
        StackMapTable   stack_map_table;
        NestMember      nest_member;
        InnerClasses    inner_classes;
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
    u16        attribute_count;
} Method;

typedef struct {
    union {
        u32        u32;
        u16        u16;
        Constant   constant;
        Method     method;
        Attribute* attribute;
    };
    Tag tag;
} Token;

typedef struct {
    u32              file_size;
    u32              byte_index;
    u8               bytes[COUNT_BYTES];
    u32              token_index;
    Token            tokens[COUNT_TOKENS];
    u32              char_index;
    char             chars[COUNT_CHARS];
    const char*      utf8s_by_index[COUNT_UTF8S];
    u32              attribute_index;
    Attribute        attributes[COUNT_ATTRIBS];
    u32              line_number_entry_index;
    LineNumberEntry  line_number_entries[COUNT_LINE_NUMBER_ENTRIES];
    u32              stack_map_entry_index;
    StackMapEntry    stack_map_entries[COUNT_STACK_MAP_ENTRIES];
    u32              verification_type_index;
    VerificationType verification_types[COUNT_VERIFICATION_TYPES];
    u32              nest_member_class_index;
    u16              nest_member_classes[COUNT_NEST_MEMBER_CLASSES];
    u32              inner_class_entry_index;
    InnerClassEntry  inner_class_entries[COUNT_INNER_CLASS_ENTRIES];
} Memory;

#define OUT_OF_BOUNDS                               \
    {                                               \
        fprintf(stderr, "[ERROR] Out of bounds\n"); \
        exit(EXIT_FAILURE);                         \
    }

void set_file_to_bytes(Memory*, const char*);

u8  pop_u8(Memory*);
u8* pop_u8_ref(Memory*);
u16 pop_u16(Memory*);
u32 pop_u32(Memory*);

u8  pop_u8_at(const u8*, u32*, u32);
u16 pop_u16_at(const u8*, u32*, u32);

Token*            alloc_token(Memory*);
Attribute*        alloc_attribute(Memory*);
LineNumberEntry*  alloc_line_number_entry(Memory*);
StackMapEntry*    alloc_stack_map_entry(Memory*);
VerificationType* alloc_verification_type(Memory*);
u16*              alloc_nest_member_class(Memory*);
InnerClassEntry*  alloc_inner_class_entry(Memory*);

void push_tag_u16(Memory*, Tag, u16);

VerificationType* get_verification_type(Memory*);
Attribute*        get_attribute(Memory*);

void set_tokens(Memory*);

#endif
