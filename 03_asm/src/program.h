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
        u16                 name_index;
        u16                 string_index;
        ConstantNameAndType name_and_type;
        ConstantRef         ref;
        const char*         string;
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
    u8 left;
    u8 right;
} Args;

typedef struct {
    union {
        u16  arg;
        Args args;
    };
    OpTag tag;
} Op;

typedef struct {
    Op* ops;
    u16 max_stack;
    u16 max_local;
    u16 op_count;
    u16 exception_count;
    u16 attribute_count;
} Code;

typedef struct {
    Code code;
    u16  access_flags;
    u16  name_index;
    u16  type_index;
    u16  attribute_count;
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

static void print_program(Program* program) {
    printf("program->major_version : %hu\n"
           "program->minor_version : %hu\n"
           "\n"
           "program->constant_count : %hu\n",
           program->major_version,
           program->minor_version,
           program->constant_count);
    for (u16 i = 1; i < program->constant_count; ++i) {
        Constant constant = program->constants[i - 1];
        switch (constant.tag) {
        case CONST_CLASS: {
            printf(LINE_FMT "Class        %hu\n", i, constant.name_index);
            break;
        }
        case CONST_FIELD_REF: {
            printf(LINE_FMT "FieldRef     " U16_U16_FMT,
                   i,
                   constant.ref.class_index,
                   constant.ref.name_and_type_index);
            break;
        }
        case CONST_METHOD_REF: {
            printf(LINE_FMT "MethodRef    " U16_U16_FMT,
                   i,
                   constant.ref.class_index,
                   constant.ref.name_and_type_index);
            break;
        }
        case CONST_NAME_AND_TYPE: {
            printf(LINE_FMT "NameAndType  " U16_U16_FMT,
                   i,
                   constant.name_and_type.name_index,
                   constant.name_and_type.type_index);
            break;
        }
        case CONST_STRING: {
            printf(LINE_FMT "String       %hu\n", i, constant.string_index);
            break;
        }
        case CONST_UTF8: {
            printf(LINE_FMT "Utf8         \"%s\"\n", i, constant.string);
            break;
        }
        }
    }
    printf("\n"
           "program->access_flags : 0x%X\n"
           "\n"
           "program->this_class  : %hu\n"
           "program->super_class : %hu\n"
           "\n"
           "program->interface_count : %hu\n"
           "program->field_count     : %hu\n"
           "program->method_count    : %hu\n",
           program->access_flags,
           program->this_class,
           program->super_class,
           program->interface_count,
           program->field_count,
           program->method_count);
    for (u16 i = 0; i < program->method_count; ++i) {
        Method method = program->methods[i];
        printf("\n"
               "program->methods[%hu].access_flags         : %hu\n"
               "                   .name_index           : %hu\n"
               "                   .type_index           : %hu\n"
               "                   .code.max_stack       : %hu\n"
               "                   .code.max_local       : %hu\n"
               "                   .code.exception_count : %hu\n"
               "                   .code.attribute_count : %hu\n"
               "                   .code.op_count        : %hu\n",
               i,
               method.access_flags,
               method.name_index,
               method.type_index,
               method.code.max_stack,
               method.code.max_local,
               method.code.exception_count,
               method.code.attribute_count,
               method.code.op_count);
    }
    printf("\nprogram->attribute_count : %hu\n", program->attribute_count);
}

#define WRITE_ERROR ERROR("Unable to write to file")

static void serialize_u32(File* file, u32 bytes) {
    if (fwrite(&bytes, sizeof(u32), 1, file) != 1) {
        WRITE_ERROR;
    }
}

static void serialize_u16(File* file, u16 bytes) {
    if (fwrite(&bytes, sizeof(u16), 1, file) != 1) {
        WRITE_ERROR;
    }
}

static void serialize_program(Program* program, const char* filename) {
    File* file = fopen(filename, "wb");
    if (file == NULL) {
        ERROR("Unable to open file");
    }
    serialize_u32(file, __builtin_bswap32(0xCAFEBABE));
    serialize_u16(file, __builtin_bswap16(program->minor_version));
    serialize_u16(file, __builtin_bswap16(program->major_version));
    fclose(file);
}

#endif
