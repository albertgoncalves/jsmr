#ifndef __PROGRAM_C__
#define __PROGRAM_C__

#include "program.h"

void print_program(Program* program) {
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
               "                   .code.op_count        : %hu\n",
               i,
               method.access_flags,
               method.name_index,
               method.type_index,
               method.code.max_stack,
               method.code.max_local,
               method.code.op_count);
    }
    printf("\nprogram->attribute_count : %hu\n", program->attribute_count);
}

#define WRITE_ERROR ERROR("Unable to write to file")

void serialize_u8(File* file, u8 bytes) {
    if (fwrite(&bytes, sizeof(u8), 1, file) != 1) {
        WRITE_ERROR;
    }
}

void serialize_u16(File* file, u16 bytes) {
    u16 swap_bytes = __builtin_bswap16(bytes);
    if (fwrite(&swap_bytes, sizeof(u16), 1, file) != 1) {
        WRITE_ERROR;
    }
}

void serialize_u32(File* file, u32 bytes) {
    u32 swap_bytes = __builtin_bswap32(bytes);
    if (fwrite(&swap_bytes, sizeof(u32), 1, file) != 1) {
        WRITE_ERROR;
    }
}

void serialize_string(File* file, const char* bytes) {
    u16 n = get_len(bytes);
    if (fwrite(bytes, sizeof(char), n, file) != n) {
        WRITE_ERROR;
    }
}

void serialize_constants(File* file, Program* program) {
    serialize_u16(file, program->constant_count);
    for (u16 i = 1; i < program->constant_count; ++i) {
        Constant constant = program->constants[i - 1];
        switch (constant.tag) {
        case CONST_CLASS: {
            serialize_u8(file, 7);
            serialize_u16(file, constant.name_index);
            break;
        }
        case CONST_FIELD_REF: {
            serialize_u8(file, 9);
            serialize_u16(file, constant.ref.class_index);
            serialize_u16(file, constant.ref.name_and_type_index);
            break;
        }
        case CONST_METHOD_REF: {
            serialize_u8(file, 10);
            serialize_u16(file, constant.ref.class_index);
            serialize_u16(file, constant.ref.name_and_type_index);
            break;
        }
        case CONST_NAME_AND_TYPE: {
            serialize_u8(file, 12);
            serialize_u16(file, constant.name_and_type.name_index);
            serialize_u16(file, constant.name_and_type.type_index);
            break;
        }
        case CONST_STRING: {
            serialize_u8(file, 8);
            serialize_u16(file, constant.string_index);
            break;
        }
        case CONST_UTF8: {
            serialize_u8(file, 1);
            serialize_u16(file, get_len(constant.string));
            serialize_string(file, constant.string);
            break;
        }
        }
    }
}

void serialize_interfaces(File* file, Program* program) {
    serialize_u16(file, program->interface_count);
    for (u16 i = 0; i < program->interface_count; ++i) {
        NOT_IMPLEMENTED
    }
}

void serialize_fields(File* file, Program* program) {
    serialize_u16(file, program->field_count);
    for (u16 i = 0; i < program->field_count; ++i) {
        NOT_IMPLEMENTED
    }
}

u16 get_constant_utf8_index(Program* program, const char* utf8) {
    for (u16 i = 1; i < program->constant_count; ++i) {
        Constant constant = program->constants[i - 1];
        if ((constant.tag == CONST_UTF8) && (get_eq(utf8, constant.string))) {
            return i;
        }
    }
    ERROR("Constant index not found");
}

void serialize_op(File* file, Op op) {
    serialize_u8(file, (u8)op.tag);
    switch (op.tag) {
    case OP_ICONST0: {
        break;
    }
    case OP_LDC: {
        serialize_u8(file, op.u8);
        break;
    }
    case OP_RETURN: {
        break;
    }
    case OP_GETSTATIC: {
        serialize_u16(file, op.u16);
        break;
    }
    case OP_INVOKEVIRTUAL: {
        serialize_u16(file, op.u16);
        break;
    }
    }
}

void serialize_methods(File* file, Program* program) {
    serialize_u16(file, program->method_count);
    for (u16 i = 0; i < program->method_count; ++i) {
        Method method = program->methods[i];
        serialize_u16(file, method.access_flags);
        serialize_u16(file, method.name_index);
        serialize_u16(file, method.type_index);
        /* NOTE: We are only serializing a single attribute (the `code` block);
         * for now, `attribute_count` is hard-coded to `1`.
         */
        serialize_u16(file, 1);
        serialize_u16(file, get_constant_utf8_index(program, "Code"));
        u32 offset_attribute_size = (u32)ftell(file);
        serialize_u32(file, 0);
        serialize_u16(file, method.code.max_stack);
        serialize_u16(file, method.code.max_local);
        u32 offset_code_size = (u32)ftell(file);
        serialize_u32(file, 0);
        for (u16 j = 0; j < method.code.op_count; ++j) {
            serialize_op(file, method.code.ops[j]);
        }
        u32 code_size =
            (u32)(((u32)ftell(file) - offset_code_size) - sizeof(u32));
        /* NOTE: Empty `method.code.exception_table`. */
        serialize_u16(file, 0);
        /* NOTE: Empty `method.code.attributes`. */
        serialize_u16(file, 0);
        u32 attribute_size =
            (u32)(((u32)ftell(file) - offset_attribute_size) - sizeof(u32));
        fseek(file, offset_attribute_size, SEEK_SET);
        serialize_u32(file, attribute_size);
        fseek(file, offset_code_size, SEEK_SET);
        serialize_u32(file, code_size);
        fseek(file, 0, SEEK_END);
    }
}

void serialize_attributes(File* file, Program* program) {
    serialize_u16(file, program->attribute_count);
    for (u16 i = 0; i < program->attribute_count; ++i) {
        NOT_IMPLEMENTED
    }
}

void serialize_program_to_file(Program* program, const char* filename) {
    File* file = fopen(filename, "wb");
    if (file == NULL) {
        ERROR("Unable to open file");
    }
    serialize_u32(file, 0xCAFEBABE);
    serialize_u16(file, program->minor_version);
    serialize_u16(file, program->major_version);
    serialize_constants(file, program);
    serialize_u16(file, program->access_flags);
    serialize_u16(file, program->this_class);
    serialize_u16(file, program->super_class);
    serialize_interfaces(file, program);
    serialize_fields(file, program);
    serialize_methods(file, program);
    serialize_attributes(file, program);
    fclose(file);
}

#endif
