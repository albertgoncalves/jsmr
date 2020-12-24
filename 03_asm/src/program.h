#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "prelude.h"

typedef enum {
    CONST_CLASS,
    CONST_FIELD_REF,
    CONST_METHOD_REF,
    CONST_NAME_AND_TYPE,
    CONST_STRING,
    CONST_UTF8,
} ConstantTag;

typedef struct {
    u16 name_index;
    u16 type_index;
} ConstantNameAndType;

typedef struct {
    u16 class_index;
    u16 name_and_type_index;
} ConstantRef;

typedef struct {
    union {
        const char*         string;
        ConstantNameAndType name_and_type;
        ConstantRef         ref;
        u16                 name_index;
        u16                 string_index;
    };
    ConstantTag tag;
} Constant;

typedef enum {
    OP_ICONST_0 = 3,
    OP_ICONST_1 = 4,
    OP_ICONST_2 = 5,
    OP_BIPUSH = 16,
    OP_LDC = 18,
    OP_ILOAD = 21,
    OP_ILOAD_0 = 26,
    OP_ILOAD_1 = 27,
    OP_ILOAD_2 = 28,
    OP_ILOAD_3 = 29,
    OP_ISTORE = 54,
    OP_ISTORE_1 = 60,
    OP_ISTORE_2 = 61,
    OP_ISTORE_3 = 62,
    OP_IADD = 96,
    OP_IINC = 132,
    OP_IFNE = 154,
    OP_IF_ICMPNE = 160,
    OP_IF_ICMPGE = 162,
    OP_GOTO = 167,
    OP_IRETURN = 172,
    OP_RETURN = 177,
    OP_GETSTATIC = 178,
    OP_INVOKEVIRTUAL = 182,
    OP_INVOKESTATIC = 184,
} OpTag;

typedef struct {
    u8 u8;
    i8 i8;
} Pair;

typedef struct {
    union {
        Pair pair;
        u16  u16;
        i16  i16;
        u8   u8;
        i8   i8;
    };
    OpTag tag;
} Op;

typedef struct {
    Op* ops;
    u16 max_stack;
    u16 max_local;
    u16 op_count;
} Code;

typedef struct {
    Code code;
    u16  access_flags;
    u16  name_index;
    u16  type_index;
} Method;

typedef struct {
    const Constant* constants;
    const Method*   methods;
    u16             major_version;
    u16             minor_version;
    u16             constant_count;
    u16             access_flags;
    u16             this_class;
    u16             super_class;
    u16             interface_count;
    u16             field_count;
    u16             method_count;
    u16             attribute_count;
} Program;

#define LINE_FMT    "%4hu. "
#define U16_U16_FMT "%-4hu%hu\n"

void print_program(Program*);

u16 get_constant_utf8_index(Program*, const char*);

void serialize_u8(File*, u8);
void serialize_i8(File*, i8);
void serialize_u16(File*, u16);
void serialize_i16(File*, i16);
void serialize_u32(File*, u32);

void serialize_string(File*, const char*);
void serialize_op(File*, Op);

void serialize_constants(File*, Program*);
void serialize_interfaces(File*, Program*);
void serialize_fields(File*, Program*);
void serialize_methods(File*, Program*);
void serialize_attributes(File*, Program*);

void serialize_program_to_file(Program*, const char*);

#endif
