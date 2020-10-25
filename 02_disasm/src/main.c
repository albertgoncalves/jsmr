#include "prelude.h"

#include "memory.h"

Attribute* get_attribute(Memory*);
Attribute* get_attribute(Memory* memory) {
    Attribute* attribute = alloc_attribute(memory);
    u16        attribute_name_index = pop_u16(memory);
    u32        attribute_size = pop_u32(memory);
    attribute->name_index = attribute_name_index;
    attribute->size = attribute_size;
    attribute->next_attribute = NULL;
    const char* attribute_name = memory->utf8s_by_index[attribute_name_index];
    if (!strcmp(attribute_name, "Code")) {
        attribute->tag = ATTRIB_CODE;
        attribute->code.max_stack = pop_u16(memory);
        attribute->code.max_locals = pop_u16(memory);
        u32 bytes_count = pop_u32(memory);
        attribute->code.bytes_count = bytes_count;
        if (bytes_count != 0) {
            attribute->code.bytes = pop_u8_ref(memory);
            memory->byte_index += (bytes_count - 1);
        }
        u16 exception_table_count = pop_u16(memory);
        attribute->code.exception_table_count = exception_table_count;
        if (exception_table_count != 0) {
            fprintf(stderr,
                    "[ERROR] `{ ? exception_table }` unimplemented\n\n");
        }
        u16 attributes_count = pop_u16(memory);
        attribute->code.attributes_count = attributes_count;
        Attribute* code_attribute = NULL;
        Attribute* code_prev_attribute = NULL;
        for (u16 i = 0; i < attributes_count; ++i) {
            code_attribute = get_attribute(memory);
            if (i == 0) {
                attribute->code.attributes = code_attribute;
            }
            if (code_prev_attribute != NULL) {
                code_prev_attribute->next_attribute = code_attribute;
            }
            code_prev_attribute = code_attribute;
        }
    } else {
        fprintf(stderr,
                "[DEBUG] %hu\n[DEBUG] %s\n",
                attribute_name_index,
                attribute_name);
        fprintf(stderr, "[ERROR] `{ ? method }` unimplemented\n\n");
        exit(EXIT_FAILURE);
    }
    return attribute;
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
        u16 constant_pool_count = pop_u16(memory);
        push_tag_u16(memory, CONSTANT_POOL_COUNT, constant_pool_count);
        for (u16 i = 1; i < constant_pool_count; ++i) {
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
                memory->utf8s_by_index[i] = utf8;
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
        u16 interfaces_count = pop_u16(memory);
        push_tag_u16(memory, INTERFACES_COUNT, interfaces_count);
        for (u16 i = 0; i < interfaces_count; ++i) {
            fprintf(stderr, "[ERROR] `{ u16 interface }` unimplemented\n\n");
            break;
        }
    }
    {
        u16 fields_count = pop_u16(memory);
        push_tag_u16(memory, FIELDS_COUNT, fields_count);
        for (u16 i = 0; i < fields_count; ++i) {
            fprintf(stderr, "[ERROR] `{ ? field }` unimplemented\n\n");
            break;
        }
    }
    {
        u16 methods_count = pop_u16(memory);
        push_tag_u16(memory, METHODS_COUNT, methods_count);
        for (u16 i = 0; i < methods_count; ++i) {
            Token* token = alloc_token(memory);
            token->tag = METHOD;
            token->method.access_flags = pop_u16(memory);
            token->method.name_index = pop_u16(memory);
            token->method.descriptor_index = pop_u16(memory);
            u16 method_attributes_count = pop_u16(memory);
            token->method.attributes_count = method_attributes_count;
            Attribute* attribute = NULL;
            Attribute* prev_attribute = NULL;
            for (u16 j = 0; j < method_attributes_count; ++j) {
                attribute = get_attribute(memory);
                if (j == 0) {
                    token->method.attributes = attribute;
                }
                if (prev_attribute != NULL) {
                    prev_attribute->next_attribute = attribute;
                }
                prev_attribute = attribute;
            }
            break;
        }
    }
}

void print_attribute(Attribute*);
void print_attribute(Attribute* attribute) {
    printf("  %-4hu%-14u(u16 AttributeNameIndex, u32 AttributeSize)\n",
           attribute->name_index,
           attribute->size);
    switch (attribute->tag) {
    case ATTRIB_CODE: {
        printf("  %-4hu%-4hu%-10u"
               "(u16 CodeMaxStack, u16 CodeMaxLocals, u32 CodeBytesCount)\n",
               attribute->code.max_stack,
               attribute->code.max_locals,
               attribute->code.bytes_count);
        printf("  {\n");
        for (u32 k = 0; k < attribute->code.bytes_count;) {
            OpCode op_code = attribute->code.bytes[k++];
            switch (op_code) {
            case OP_ALOAD_0: {
                printf("    aload_0\n");
                break;
            case OP_INVOKESPECIAL: {
                u8  byte1 = attribute->code.bytes[k++];
                u8  byte2 = attribute->code.bytes[k++];
                u16 index = (u16)((byte1 << 8) | byte2);
                printf("    invokespecial #%hu\n", index);
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
        printf("  %-18hu(u16 CodeExceptionTableCount)\n",
               attribute->code.exception_table_count);
        printf("  %-18hu(u16 CodeAttributesCount)\n",
               attribute->code.attributes_count);
        Attribute* code_attribute = attribute->code.attributes;
        for (u16 k = 0; k < attribute->code.attributes_count; ++k) {
            if (code_attribute != NULL) {
                print_attribute(code_attribute);
                code_attribute = code_attribute->next_attribute;
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
        case CONSTANT_POOL_COUNT: {
            printf("  %-18hu(u16 ConstantPoolCount)\n\n", token.u16);
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
        case INTERFACES_COUNT: {
            printf("  %-18hu(u16 InterfacesCount)\n", token.u16);
            break;
        }
        case FIELDS_COUNT: {
            printf("  %-18hu(u16 FieldsCount)\n", token.u16);
            break;
        }
        case METHODS_COUNT: {
            printf("  %-18hu(u16 MethodsCount)\n", token.u16);
            break;
        }
        case METHOD: {
            printf("  %-4hu%-4hu%-4hu%-6hu"
                   "(u16 MethodAccessFlags, u16 MethodNameIndex, "
                   "u16 MethodDescriptorIndex, u16 MethodAttributesCount)"
                   "\n",
                   token.method.access_flags,
                   token.method.name_index,
                   token.method.descriptor_index,
                   token.method.attributes_count);
            Attribute* attribute = token.method.attributes;
            for (u16 j = 0; j < token.method.attributes_count; ++j) {
                if (attribute != NULL) {
                    print_attribute(attribute);
                    attribute = attribute->next_attribute;
                }
            }
            break;
        }
        }
    }
}

i32 main(i32 n, const char** args) {
    printf("sizeof(u8)           : %zu\n"
           "sizeof(Tag)          : %zu\n"
           "sizeof(ConstantTag)  : %zu\n"
           "sizeof(Constant)     : %zu\n"
           "sizeof(CodeAttribute): %zu\n"
           "sizeof(Attribute)    : %zu\n"
           "sizeof(Method)       : %zu\n"
           "sizeof(Token)        : %zu\n"
           "sizeof(Memory)       : %zu\n"
           "\n",
           sizeof(u8),
           sizeof(Tag),
           sizeof(ConstantTag),
           sizeof(Constant),
           sizeof(CodeAttribute),
           sizeof(Attribute),
           sizeof(Method),
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
