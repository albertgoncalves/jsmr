#ifndef __MEMORY_C__
#define __MEMORY_C__

#include "memory.h"

void set_file_to_bytes(Memory* memory, const char* filename) {
    File* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "[ERROR] Unable to open file\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    u32 file_size = (u32)ftell(file);
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

u8 pop_u8(Memory* memory) {
    if (memory->file_size <= memory->byte_index) {
        OUT_OF_BOUNDS;
    }
    return memory->bytes[memory->byte_index++];
}

u8* pop_u8_ref(Memory* memory) {
    if (memory->file_size <= memory->byte_index) {
        OUT_OF_BOUNDS;
    }
    return &memory->bytes[memory->byte_index++];
}

u16 pop_u16(Memory* memory) {
    u32 next_index = memory->byte_index + 2;
    if (memory->file_size < next_index) {
        OUT_OF_BOUNDS;
    }
    u32 i = memory->byte_index;
    u16 bytes = (u16)((memory->bytes[i] << 8) | (memory->bytes[i + 1]));
    memory->byte_index = next_index;
    return bytes;
}

u8 pop_u8_at(const u8* bytes, u32* index, u32 size) {
    if (size < ((*index) + 1)) {
        OUT_OF_BOUNDS;
    }
    return bytes[(*index)++];
}

u16 pop_u16_at(const u8* bytes, u32* index, u32 size) {
    if (size < ((*index) + 2)) {
        OUT_OF_BOUNDS;
    }
    u8 byte1 = bytes[(*index)++];
    u8 byte2 = bytes[(*index)++];
    return (u16)((byte1 << 8) | byte2);
}

u32 pop_u32(Memory* memory) {
    u32 next_index = memory->byte_index + 4;
    if (memory->file_size < next_index) {
        OUT_OF_BOUNDS;
    }
    u32 i = memory->byte_index;
    u32 bytes = (u32)((memory->bytes[i] << 24) | (memory->bytes[i + 1] << 16) |
                      (memory->bytes[i + 2] << 8) | memory->bytes[i + 3]);
    memory->byte_index = next_index;
    return bytes;
}

Token* alloc_token(Memory* memory) {
    if (COUNT_TOKENS <= memory->token_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new token\n");
        exit(EXIT_FAILURE);
    }
    return &memory->tokens[memory->token_index++];
}

Attribute* alloc_attribute(Memory* memory) {
    if (COUNT_ATTRIBS <= memory->attribute_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new attribute\n");
        exit(EXIT_FAILURE);
    }
    Attribute* attribute = &memory->attributes[memory->attribute_index++];
    attribute->next_attribute = NULL;
    return attribute;
}

LineNumberEntry* alloc_line_number_entry(Memory* memory) {
    if (COUNT_LINE_NUMBER_ENTRIES <= memory->line_number_entry_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new line number entry\n");
        exit(EXIT_FAILURE);
    }
    return &memory->line_number_entries[memory->line_number_entry_index++];
}

StackMapEntry* alloc_stack_map_entry(Memory* memory) {
    if (COUNT_STACK_MAP_ENTRIES <= memory->stack_map_entry_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new stack map entry\n");
        exit(EXIT_FAILURE);
    }
    return &memory->stack_map_entries[memory->stack_map_entry_index++];
}

VerificationType* alloc_verification_type(Memory* memory) {
    if (COUNT_VERIFICATION_TYPES <= memory->verification_type_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new verification type\n");
        exit(EXIT_FAILURE);
    }
    return &memory->verification_types[memory->verification_type_index++];
}

u16* alloc_nest_member_class(Memory* memory) {
    if (COUNT_NEST_MEMBER_CLASSES <= memory->nest_member_class_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new nest member class\n");
        exit(EXIT_FAILURE);
    }
    return &memory->nest_member_classes[memory->nest_member_class_index++];
}

InnerClassEntry* alloc_inner_class_entry(Memory* memory) {
    if (COUNT_INNER_CLASS_ENTRIES <= memory->inner_class_entry_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new inner class entry\n");
        exit(EXIT_FAILURE);
    }
    return &memory->inner_class_entries[memory->inner_class_entry_index++];
}

void push_tag_u16(Memory* memory, Tag tag, u16 value) {
    Token* token = alloc_token(memory);
    token->tag = tag;
    token->u16 = value;
}

VerificationType* get_verification_type(Memory* memory) {
    VerificationType* verification_type = alloc_verification_type(memory);
    u8                bit_tag = pop_u8(memory);
    verification_type->bit_tag = bit_tag;
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

Attribute* get_attribute(Memory* memory) {
    Attribute* attribute = alloc_attribute(memory);
    u16        attribute_name_index = pop_u16(memory);
    u32        attribute_size = pop_u32(memory);
    attribute->name_index = attribute_name_index;
    attribute->size = attribute_size;
    attribute->next_attribute = NULL;
    const char* attribute_name = memory->utf8s_by_index[attribute_name_index];
    if (get_eq(attribute_name, "Code")) {
        attribute->tag = ATTRIB_CODE;
        attribute->code.max_stack = pop_u16(memory);
        attribute->code.max_local = pop_u16(memory);
        u32 byte_count = pop_u32(memory);
        attribute->code.byte_count = byte_count;
        if (byte_count != 0) {
            attribute->code.bytes = pop_u8_ref(memory);
            memory->byte_index += (byte_count - 1);
        }
        u16 exception_table_count = pop_u16(memory);
        attribute->code.exception_table_count = exception_table_count;
        if (exception_table_count != 0) {
            fprintf(stderr,
                    "[ERROR] `{ ? exception_table }` unimplemented\n\n");
            exit(EXIT_FAILURE);
        }
        u16 attribute_count = pop_u16(memory);
        attribute->code.attribute_count = attribute_count;
        Attribute* code_attribute = NULL;
        Attribute* code_prev_attribute = NULL;
        for (u16 i = 0; i < attribute_count; ++i) {
            code_attribute = get_attribute(memory);
            if (i == 0) {
                attribute->code.attributes = code_attribute;
            }
            if (code_prev_attribute != NULL) {
                code_prev_attribute->next_attribute = code_attribute;
            }
            code_prev_attribute = code_attribute;
        }
    } else if (get_eq(attribute_name, "LineNumberTable")) {
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
    } else if (get_eq(attribute_name, "StackMapTable")) {
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
                stack_map_entry->stack_item_count = 1;
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
                fprintf(stderr,
                        "[ERROR] `{ u8 stack_map_bit_tag (%hhu) }` "
                        "unimplemented\n\n",
                        bit_tag);
                exit(EXIT_FAILURE);
            }
        }
    } else if (get_eq(attribute_name, "SourceFile")) {
        attribute->tag = ATTRIB_SOURCE_FILE;
        attribute->u16 = pop_u16(memory);
    } else if (get_eq(attribute_name, "NestMembers")) {
        attribute->tag = ATTRIB_NEST_MEMBER;
        u16 nest_member_count = pop_u16(memory);
        attribute->nest_member.count = nest_member_count;
        for (u16 i = 0; i < nest_member_count; ++i) {
            u16* nest_member_class = alloc_nest_member_class(memory);
            if (i == 0) {
                attribute->nest_member.classes = nest_member_class;
            }
            *nest_member_class = pop_u16(memory);
        }
    } else if (get_eq(attribute_name, "InnerClasses")) {
        attribute->tag = ATTRIB_INNER_CLASSES;
        u16 inner_classes_count = pop_u16(memory);
        attribute->inner_classes.count = inner_classes_count;
        for (u16 i = 0; i < inner_classes_count; ++i) {
            InnerClassEntry* inner_class_entry =
                alloc_inner_class_entry(memory);
            if (i == 0) {
                attribute->inner_classes.entries = inner_class_entry;
            }
            inner_class_entry->inner_class_info_index = pop_u16(memory);
            inner_class_entry->outer_class_info_index = pop_u16(memory);
            inner_class_entry->inner_name_index = pop_u16(memory);
            inner_class_entry->inner_class_access_flags = pop_u16(memory);
        }
    } else {
        fprintf(stderr,
                "[DEBUG] %hu\n[DEBUG] %s\n"
                "[ERROR] `{ ? attribute }` unimplemented\n\n",
                attribute_name_index,
                attribute_name);
        exit(EXIT_FAILURE);
    }
    return attribute;
}

void set_tokens(Memory* memory) {
    memory->byte_index = 0;
    memory->token_index = 0;
    memory->char_index = 0;
    memory->attribute_index = 0;
    memory->line_number_entry_index = 0;
    memory->stack_map_entry_index = 0;
    memory->verification_type_index = 0;
    {
        u32 magic = pop_u32(memory);
        if (magic != 0xCAFEBABE) {
            printf("[ERROR] Incorrect magic constant\n");
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
                    fprintf(stderr, "[ERROR] Unable to allocate string\n");
                    exit(EXIT_FAILURE);
                }
                if (COUNT_UTF8S <= i) {
                    fprintf(stderr,
                            "[ERROR] Unable to allocate UTF8 pointer\n");
                    exit(EXIT_FAILURE);
                }
                token->constant.utf8.size = utf8_size;
                const char* utf8 = &memory->chars[memory->char_index];
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
                printf("[ERROR] `{ ConstantTag tag (%hhu) }` "
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
        u16 interface_count = pop_u16(memory);
        push_tag_u16(memory, INTERFACE_COUNT, interface_count);
        for (u16 i = 0; i < interface_count; ++i) {
            printf("[ERROR] `{ u16 interface }` unimplemented\n\n");
            return;
        }
    }
    {
        u16 field_count = pop_u16(memory);
        push_tag_u16(memory, FIELD_COUNT, field_count);
        for (u16 i = 0; i < field_count; ++i) {
            printf("[ERROR] `{ ? field }` unimplemented\n\n");
            return;
        }
    }
    {
        u16 method_count = pop_u16(memory);
        push_tag_u16(memory, METHOD_COUNT, method_count);
        for (u16 i = 0; i < method_count; ++i) {
            Token* token = alloc_token(memory);
            token->tag = METHOD;
            token->method.access_flags = pop_u16(memory);
            token->method.name_index = pop_u16(memory);
            token->method.descriptor_index = pop_u16(memory);
            u16 method_attribute_count = pop_u16(memory);
            token->method.attribute_count = method_attribute_count;
            Attribute* attribute = NULL;
            Attribute* prev_attribute = NULL;
            for (u16 j = 0; j < method_attribute_count; ++j) {
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
    {
        u16 attribute_count = pop_u16(memory);
        {
            Token* token = alloc_token(memory);
            token->tag = ATTRIBUTE_COUNT;
            token->u16 = attribute_count;
        }
        for (u16 _ = 0; _ < attribute_count; ++_) {
            Token* token = alloc_token(memory);
            token->tag = ATTRIBUTE;
            token->attribute = get_attribute(memory);
        }
    }
}

#endif
