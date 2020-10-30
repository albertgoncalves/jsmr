#ifndef __PROGRAM_H__
#define __PROGRAM_H__

typedef enum {
    CONST_CLASS,
    CONST_NAME_AND_TYPE,
    CONST_STRING,
    CONST_UTF8,
} ConstantTag;

typedef struct {
    u16 name_index;
    u16 type_index;
} NameAndType;

typedef struct {
    union {
        u16         name_index;
        u16         string_index;
        NameAndType name_and_type;
        const char* string;
    };
    ConstantTag tag;
} Constant;

typedef struct {
    const Constant* constants;
    u16             major_version;
    u16             minor_version;
    u16             constant_count;
    u16             access_flags;
    u16             this_class;
    u16             super_class;
} Program;

static void print_program(Program* program) {
    printf("program->major_version  : %hu\n"
           "program->minor_version  : %hu\n"
           "program->constant_count : %hu\n",
           program->major_version,
           program->minor_version,
           program->constant_count);
    for (u16 i = 1; i < program->constant_count; ++i) {
        Constant constant = program->constants[i - 1];
        switch (constant.tag) {
        case CONST_CLASS: {
            printf("%4hu. Class        %hu\n", i, constant.name_index);
            break;
        }
        case CONST_NAME_AND_TYPE: {
            printf("%4hu. NameAndType  %hu %hu\n",
                   i,
                   constant.name_and_type.name_index,
                   constant.name_and_type.type_index);
            break;
        }
        case CONST_STRING: {
            printf("%4hu. String       %hu\n", i, constant.string_index);
            break;
        }
        case CONST_UTF8: {
            printf("%4hu. Utf8         \"%s\"\n", i, constant.string);
            break;
        }
        }
    }
    printf("program->access_flags : 0x%X\n"
           "program->this_class   : %hu\n"
           "program->super_class  : %hu\n",
           program->access_flags,
           program->this_class,
           program->super_class);
}

#endif
