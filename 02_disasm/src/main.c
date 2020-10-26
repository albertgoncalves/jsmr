#include "prelude.h"

#include "memory.h"

static VerificationType* get_verification_type(Memory* memory) {
    VerificationType*   verification_type = alloc_verification_type(memory);
    u8                  bit_tag = pop_u8(memory);
    VerificationTypeTag tag = (VerificationTypeTag)bit_tag;
    switch (tag) {
    case VERI_TOP:
    case VERI_INTEGER:
    case VERI_FLOAT:
    case VERI_DOUBLE:
    case VERI_LONG:
    case VERI_NULL:
    case VERI_UNINIT_THIS: {
        break;
    }
    case VERI_OBJECT: {
        verification_type->constant_pool_index = pop_u16(memory);
        break;
    }
    case VERI_UNINIT: {
        verification_type->offset = pop_u16(memory);
        break;
    }
    }
    return verification_type;
}

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
            fprintf(stdout,
                    "[ERROR] `{ ? exception_table }` unimplemented\n\n");
            exit(EXIT_FAILURE);
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
    } else if (!strcmp(attribute_name, "LineNumberTable")) {
        attribute->tag = ATTRIB_LINE_NUMBER_TABLE;
        u16 line_number_table_count = pop_u16(memory);
        attribute->line_number_table.count = line_number_table_count;
        for (u16 i = 0; i < line_number_table_count; ++i) {
            LineNumberEntry* line_number_entry =
                alloc_line_number_entry(memory);
            if (i == 0) {
                attribute->line_number_table.entries = line_number_entry;
            }
            line_number_entry->pc_start = pop_u16(memory);
            line_number_entry->line_number = pop_u16(memory);
        }
    } else if (!strcmp(attribute_name, "StackMapTable")) {
        attribute->tag = ATTRIB_STACK_MAP_TABLE;
        u16 stack_map_table_count = pop_u16(memory);
        attribute->stack_map_table.count = stack_map_table_count;
        for (u16 i = 0; i < stack_map_table_count; ++i) {
            StackMapEntry* stack_map_entry = alloc_stack_map_entry(memory);
            if (i == 0) {
                attribute->stack_map_table.entries = stack_map_entry;
            }
            u8 bit_tag = pop_u8(memory);
            stack_map_entry->bit_tag = bit_tag;
            if (bit_tag < 64) {
                stack_map_entry->tag = STACK_MAP_SAME_FRAME;
            } else if (bit_tag < 128) {
                stack_map_entry->tag =
                    STACK_MAP_SAME_LOCALS_1_STACK_ITEM_FRAME;
                stack_map_entry->stack_items = get_verification_type(memory);
            } else if (bit_tag == 255) {
                stack_map_entry->tag = STACK_MAP_FULL_FRAME;
                stack_map_entry->offset_delta = pop_u16(memory);
                u16 local_item_count = pop_u16(memory);
                stack_map_entry->local_item_count = local_item_count;
                for (u16 j = 0; j < local_item_count; ++j) {
                    VerificationType* verification_type =
                        get_verification_type(memory);
                    if (j == 0) {
                        stack_map_entry->local_items = verification_type;
                    }
                }
                u16 stack_item_count = pop_u16(memory);
                stack_map_entry->stack_item_count = stack_item_count;
                for (u16 j = 0; j < stack_item_count; ++j) {
                    VerificationType* verification_type =
                        get_verification_type(memory);
                    if (j == 0) {
                        stack_map_entry->stack_items = verification_type;
                    }
                }
            } else if ((248 <= bit_tag) && (bit_tag < 251)) {
                stack_map_entry->tag = STACK_MAP_CHOP_FRAME;
                stack_map_entry->offset_delta = pop_u16(memory);
            } else if ((252 <= bit_tag) && (bit_tag < 255)) {
                stack_map_entry->tag = STACK_MAP_APPEND_FRAME;
                stack_map_entry->offset_delta = pop_u16(memory);
                u16 local_item_count = (u16)(bit_tag - 251);
                stack_map_entry->local_item_count = local_item_count;
                for (u16 j = 0; j < local_item_count; ++j) {
                    VerificationType* verification_type =
                        get_verification_type(memory);
                    if (j == 0) {
                        stack_map_entry->local_items = verification_type;
                    }
                }
            } else {
                fprintf(stdout,
                        "[ERROR] `{ u8 stack_map_bit_tag (%hhu) }` "
                        "unimplemented\n\n",
                        bit_tag);
                exit(EXIT_FAILURE);
            }
        }
    } else {
        fprintf(stdout,
                "[DEBUG] %hu\n[DEBUG] %s\n",
                attribute_name_index,
                attribute_name);
        fprintf(stdout, "[ERROR] `{ ? attribute }` unimplemented\n\n");
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
            fprintf(stdout, "[ERROR] Incorrect magic constant\n");
            return;
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
                if (COUNT_CHARS <= (memory->char_index + utf8_size + 1)) {
                    fprintf(stdout, "[ERROR] Unable to allocate string\n");
                    exit(EXIT_FAILURE);
                }
                if (COUNT_UTF8S <= i) {
                    fprintf(stdout,
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
                fprintf(stdout,
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
            fprintf(stdout, "[ERROR] `{ u16 interface }` unimplemented\n\n");
            return;
        }
    }
    {
        u16 fields_count = pop_u16(memory);
        push_tag_u16(memory, FIELDS_COUNT, fields_count);
        for (u16 i = 0; i < fields_count; ++i) {
            fprintf(stdout, "[ERROR] `{ ? field }` unimplemented\n\n");
            return;
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
        }
    }
}

#define OP_OFFSET      "%-14s"
#define FMT_OP_U8      OP_OFFSET "%hhu\n"
#define FMT_OP_U16     OP_OFFSET "%hu\n"
#define FMT_OP_I16     OP_OFFSET "%hd\n"
#define FMT_OP_U8_I8   OP_OFFSET "%-6hhu%hhd\n"

void print_attribute(Attribute*);
void print_attribute(Attribute* attribute) {
    printf("\n  %-4hu%-14u(u16 AttributeNameIndex, u32 AttributeSize)\n"
           "                    ",
           attribute->name_index,
           attribute->size);
    switch (attribute->tag) {
    case ATTRIB_CODE: {
        printf("[ CodeAttribute ]\n");
        printf("  %-4hu%-4hu%-10u"
               "(u16 CodeMaxStack, u16 CodeMaxLocals, u32 CodeBytesCount)\n",
               attribute->code.max_stack,
               attribute->code.max_locals,
               attribute->code.bytes_count);
        printf("    {\n");
        for (u32 i = 0; i < attribute->code.bytes_count;) {
            printf("      #%-4u ", i);
            OpCode op_code = attribute->code.bytes[i++];
            switch (op_code) {
            case OP_ALOAD_0: {
                printf("aload_0\n");
                break;
            case OP_INVOKESPECIAL: {
                printf(FMT_OP_U16,
                       "invokespecial",
                       pop_u16_at(attribute->code.bytes, &i));
                break;
            }
            case OP_RETURN: {
                printf("return\n");
                break;
            }
            case OP_GETFIELD: {
                printf(FMT_OP_U16,
                       "getfield",
                       pop_u16_at(attribute->code.bytes, &i));
                break;
            }
            case OP_IFNE: {
                printf(FMT_OP_I16,
                       "ifne",
                       (i16)pop_u16_at(attribute->code.bytes, &i));
                break;
            }
            case OP_ICONST_0: {
                printf("iconst_0\n");
                break;
            }
            case OP_IRETURN: {
                printf("ireturn\n");
                break;
            }
            case OP_ICONST_1: {
                printf("iconst_1\n");
                break;
            }
            case OP_IF_ICMPNE: {
                printf(FMT_OP_I16,
                       "if_icmpne",
                       (i16)pop_u16_at(attribute->code.bytes, &i));
                break;
            }
            case OP_ISTORE_1: {
                printf("istore_1\n");
                break;
            }
            case OP_ISTORE_2: {
                printf("istore_2\n");
                break;
            }
            case OP_ISTORE_3: {
                printf("istore_3\n");
                break;
            }
            case OP_ICONST_2: {
                printf("iconst_2\n");
                break;
            }
            case OP_ISTORE: {
                printf(FMT_OP_U8,
                       "istore",
                       pop_u8_at(attribute->code.bytes, &i));
                break;
            }
            case OP_ILOAD: {
                printf(FMT_OP_U8,
                       "iload",
                       pop_u8_at(attribute->code.bytes, &i));
                break;
            }
            case OP_IF_ICMPGE: {
                printf(FMT_OP_I16,
                       "if_icmpge",
                       (i16)pop_u16_at(attribute->code.bytes, &i));
                break;
            }
            case OP_ILOAD_2: {
                printf("iload_2\n");
                break;
            }
            case OP_ILOAD_3: {
                printf("iload_3\n");
                break;
            }
            case OP_ILOAD_1: {
                printf("iload_1\n");
                break;
            }
            case OP_IADD: {
                printf("iadd\n");
                break;
            }
            case OP_IINC: {
                u8 index = pop_u8_at(attribute->code.bytes, &i);
                i8 constant = (i8)pop_u8_at(attribute->code.bytes, &i);
                printf(FMT_OP_U8_I8, "iinc", index, constant);
                break;
            }
            case OP_GOTO: {
                printf(FMT_OP_I16,
                       "goto",
                       (i16)pop_u16_at(attribute->code.bytes, &i));
                break;
            }
            }
            default: {
                fprintf(stdout,
                        "[ERROR] `{ OpCode op_code (%hhu) }` "
                        "unimplemented\n\n",
                        (u8)op_code);
                exit(EXIT_FAILURE);
            }
            }
        }
        printf("    }\n");
        printf("  %-18hu(u16 CodeExceptionTableCount)\n",
               attribute->code.exception_table_count);
        printf("  %-18hu(u16 CodeAttributesCount)\n",
               attribute->code.attributes_count);
        Attribute* code_attribute = attribute->code.attributes;
        for (u16 _ = 0; _ < attribute->code.attributes_count; ++_) {
            if (code_attribute != NULL) {
                print_attribute(code_attribute);
                code_attribute = code_attribute->next_attribute;
            }
        }
        break;
    }
    case ATTRIB_LINE_NUMBER_TABLE: {
        printf("[ LineNumberTableAttribute ]\n");
        u16 line_number_table_count = attribute->line_number_table.count;
        printf("  %-18hu(u16 LineNumberTableCount)\n",
               line_number_table_count);
        for (u16 i = 0; i < line_number_table_count; ++i) {
            LineNumberEntry line_number_entry =
                attribute->line_number_table.entries[i];
            printf("  %-4hu%-14hu(u16 PcStart, u16 LineNumber)\n",
                   line_number_entry.pc_start,
                   line_number_entry.line_number);
        }
        break;
    }
    case ATTRIB_STACK_MAP_TABLE: {
        printf("[ StackMapTableAttribute ]\n");
        fprintf(stdout, "[ERROR] StackMapTableAttribute not printable\n");
        exit(EXIT_FAILURE);
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
                       "                    #%-3hu "
                       "(u8 Constant.Utf8, u16 Length, u8*%hu String)\n",
                       (u8)token.constant.tag,
                       token.constant.utf8.size,
                       token.constant.utf8.string,
                       token.constant.index,
                       token.constant.utf8.size);
                break;
            }
            case CONSTANT_TAG_CLASS: {
                printf("  %-4hhu%-14hu#%-3hu (u8 Constant.Class, "
                       "u16 NameIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.class_.name_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_STRING: {
                printf("  %-4hhu%-14hu#%-3hu (u8 Constant.String, "
                       "u16 StringIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.string.string_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_FIELD_REF: {
                printf("  %-4hhu%-4hu%-10hu#%-3hu "
                       "(u8 Constant.FieldRef, u16 ClassIndex, "
                       "u16 NameAndTypeIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.ref.class_index,
                       token.constant.ref.name_and_type_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_METHOD_REF: {
                printf("  %-4hhu%-4hu%-10hu#%-3hu "
                       "(u8 Constant.MethodRef, u16 ClassIndex,\n"
                       "                          "
                       "u16 NameAndTypeIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.ref.class_index,
                       token.constant.ref.name_and_type_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_NAME_AND_TYPE: {
                printf("  %-4hhu%-4hu%-10hu#%-3hu "
                       "(u8 Constant.NameAndType, u16 NameIndex,\n"
                       "                          "
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
                   "(u16 MethodAccessFlags, u16 MethodNameIndex,\n"
                   "                     "
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
    printf("sizeof(Constant)    : %zu\n"
           "sizeof(Code)        : %zu\n"
           "sizeof(Attribute)   : %zu\n"
           "sizeof(Method)      : %zu\n"
           "sizeof(Token)       : %zu\n"
           "sizeof(Memory)      : %zu\n"
           "\n",
           sizeof(Constant),
           sizeof(Code),
           sizeof(Attribute),
           sizeof(Method),
           sizeof(Token),
           sizeof(Memory));
    if (n < 2) {
        fprintf(stdout, "[ERROR] No file provided\n");
        exit(EXIT_FAILURE);
    }
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        fprintf(stdout, "[ERROR] `calloc` failed\n");
        exit(EXIT_FAILURE);
    }
    set_file_to_bytes(memory, args[1]);
    set_tokens(memory);
    print_tokens(memory);
    fprintf(stdout,
            "\n[INFO] %zu bytes left!\n",
            memory->file_size - memory->byte_index);
    free(memory);
    return EXIT_SUCCESS;
}
