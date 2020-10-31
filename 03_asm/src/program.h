#ifndef __PROGRAM_H__
#define __PROGRAM_H__

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
    OP_LDC = 18,
    OP_RETURN = 177,
    OP_GETSTATIC = 178,
    OP_INVOKEVIRTUAL = 182,
} OpTag;

typedef struct {
    union {
        u16 u16;
        u8  u8;
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
void serialize_u16(File*, u16);
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
