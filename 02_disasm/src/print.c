#ifndef __PRINT_C__
#define __PRINT_C__

#include "print.h"

void print_op_codes(const u8* bytes, u32 byte_count) {
    printf("    {\n");
    for (u32 i = 0; i < byte_count;) {
        printf("      #%-4u ", i);
        OpCode op_code = bytes[i++];
        switch (op_code) {
        case OP_ALOAD_0: {
            printf("aload_0\n");
            break;
        }
        case OP_INVOKESPECIAL: {
            printf(OP_FMT_U16,
                   "invokespecial",
                   pop_u16_at(bytes, &i, byte_count));
            break;
        }
        case OP_RETURN: {
            printf("return\n");
            break;
        }
        case OP_GETFIELD: {
            printf(OP_FMT_U16, "getfield", pop_u16_at(bytes, &i, byte_count));
            break;
        }
        case OP_IFNE: {
            printf(OP_FMT_I16, "ifne", (i16)pop_u16_at(bytes, &i, byte_count));
            break;
        }
        case OP_IRETURN: {
            printf("ireturn\n");
            break;
        }
        case OP_IF_ICMPNE: {
            printf(OP_FMT_I16,
                   "if_icmpne",
                   (i16)pop_u16_at(bytes, &i, byte_count));
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
        case OP_ICONST_0: {
            printf("iconst_0\n");
            break;
        }
        case OP_ICONST_1: {
            printf("iconst_1\n");
            break;
        }
        case OP_ICONST_2: {
            printf("iconst_2\n");
            break;
        }
        case OP_ICONST_3: {
            printf("iconst_3\n");
            break;
        }
        case OP_ISTORE: {
            printf(OP_FMT_U8, "istore", pop_u8_at(bytes, &i, byte_count));
            break;
        }
        case OP_ILOAD: {
            printf(OP_FMT_U8, "iload", pop_u8_at(bytes, &i, byte_count));
            break;
        }
        case OP_IF_ICMPGE: {
            printf(OP_FMT_I16,
                   "if_icmpge",
                   (i16)pop_u16_at(bytes, &i, byte_count));
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
        case OP_ISUB: {
            printf("isub\n");
            break;
        }
        case OP_IINC: {
            u8 index = pop_u8_at(bytes, &i, byte_count);
            i8 constant = (i8)pop_u8_at(bytes, &i, byte_count);
            printf(OP_FMT_U8_I8, "iinc", index, constant);
            break;
        }
        case OP_GOTO: {
            printf(OP_FMT_I16, "goto", (i16)pop_u16_at(bytes, &i, byte_count));
            break;
        }
        case OP_BIPUSH: {
            printf(OP_FMT_U8, "bipush", pop_u8_at(bytes, &i, byte_count));
            break;
        }
        case OP_NEW: {
            printf(OP_FMT_U16, "new", pop_u16_at(bytes, &i, byte_count));
            break;
        }
        case OP_DUP: {
            printf("dup\n");
            break;
        }
        case OP_ASTORE_2: {
            printf("astore_2\n");
            break;
        }
        case OP_ALOAD_2: {
            printf("aload_2\n");
            break;
        }
        case OP_PUTFIELD: {
            printf(OP_FMT_U16, "putfield", pop_u16_at(bytes, &i, byte_count));
            break;
        }
        case OP_GETSTATIC: {
            printf(OP_FMT_U16, "getstatic", pop_u16_at(bytes, &i, byte_count));
            break;
        }
        case OP_INVOKESTATIC: {
            printf(OP_FMT_U16,
                   "invokestatic",
                   pop_u16_at(bytes, &i, byte_count));
            break;
        }
        case OP_INVOKEVIRTUAL: {
            printf(OP_FMT_U16,
                   "invokevirtual",
                   pop_u16_at(bytes, &i, byte_count));
            break;
        }
        case OP_LDC: {
            printf(OP_FMT_U8, "ldc", pop_u8_at(bytes, &i, byte_count));
            break;
        }
        case OP_ILOAD_0: {
            printf("iload_0\n");
            break;
        }
        default: {
            fflush(stdout);
            fprintf(stderr,
                    "\n[ERROR] `{ OpCode op_code (%hhu) }` "
                    "unimplemented\n\n",
                    (u8)op_code);
            exit(EXIT_FAILURE);
        }
        }
    }
    printf("    }\n");
}

void print_verification_table(VerificationType* verification_types,
                              u16               verification_type_count) {
    for (u16 i = 0; i < verification_type_count; ++i) {
        VerificationType verification_type = verification_types[i];
        switch (verification_type.tag) {
        case VERI_TOP: {
            printf(TOKEN_FMT_U8 "(u8 VerificationItem.Top)\n",
                   verification_type.bit_tag);
            break;
        }
        case VERI_INTEGER: {
            printf(TOKEN_FMT_U8 "(u8 VerificationItem.Integer)\n",
                   verification_type.bit_tag);
            break;
        }
        case VERI_FLOAT: {
            printf(TOKEN_FMT_U8 "(u8 VerificationItem.Float)\n",
                   verification_type.bit_tag);
            break;
        }
        case VERI_DOUBLE: {
            printf(TOKEN_FMT_U8 "(u8 VerificationItem.Double)\n",
                   verification_type.bit_tag);
            break;
        }
        case VERI_LONG: {
            printf(TOKEN_FMT_U8 "(u8 VerificationItem.Long)\n",
                   verification_type.bit_tag);
            break;
        }
        case VERI_NULL: {
            printf(TOKEN_FMT_U8 "(u8 VerificationItem.Null)\n",
                   verification_type.bit_tag);
            break;
        }
        case VERI_UNINIT_THIS: {
            printf(TOKEN_FMT_U8 "(u8 VerificationItem.UninitThis)\n",
                   verification_type.bit_tag);
            break;
        }
        case VERI_OBJECT: {
            printf(TOKEN_FMT_U8_U16 "(u8 VerificationItem.Object)\n",
                   verification_type.bit_tag,
                   verification_type.constant_pool_index);
            break;
        }
        case VERI_UNINIT: {
            printf(TOKEN_FMT_U8_U16 "(u8 VerificationItem.Uninit)\n",
                   verification_type.bit_tag,
                   verification_type.offset);
            break;
        }
        }
    }
}

void print_attribute(Attribute* attribute) {
    printf("\n  %-4hu%-14u(u16 AttributeNameIndex, u32 AttributeSize)\n"
           "                    ",
           attribute->name_index,
           attribute->size);
    switch (attribute->tag) {
    case ATTRIB_CODE: {
        printf("[ CodeAttribute ]\n");
        printf("  %-4hu%-4hu%-10u"
               "(u16 CodeMaxStack, u16 CodeMaxLocal, u32 CodeByteCount)\n",
               attribute->code.max_stack,
               attribute->code.max_local,
               attribute->code.byte_count);
        print_op_codes(attribute->code.bytes, attribute->code.byte_count);
        printf(TOKEN_FMT_U16 "(u16 CodeExceptionTableCount)\n",
               attribute->code.exception_table_count);
        printf(TOKEN_FMT_U16 "(u16 CodeAttributeCount)\n",
               attribute->code.attribute_count);
        Attribute* code_attribute = attribute->code.attributes;
        for (u16 _ = 0; _ < attribute->code.attribute_count; ++_) {
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
        printf(TOKEN_FMT_U16 "(u16 LineNumberTableCount)\n",
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
        for (u16 i = 0; i < attribute->stack_map_table.count; ++i) {
            StackMapEntry stack_map_entry =
                attribute->stack_map_table.entries[i];
            switch (stack_map_entry.tag) {
            case STACK_MAP_SAME_FRAME: {
                printf(TOKEN_FMT_U8 "(u8 SameFrame)\n",
                       stack_map_entry.bit_tag);
                break;
            }
            case STACK_MAP_SAME_LOCALS_1_STACK_ITEM_FRAME: {
                printf(TOKEN_FMT_U8 "(u8 SameLocals1StackItemFrame)\n",
                       stack_map_entry.bit_tag);
                print_verification_table(stack_map_entry.stack_items,
                                         stack_map_entry.stack_item_count);
                break;
            }
            case STACK_MAP_SAME_LOCALS_1_STACK_ITEM_FRAME_EXTENDED: {
                printf(TOKEN_FMT_U8_U16
                       "(u8 SameLocals1StackItemFrameExtended, "
                       "u16 OffsetDelta)\n",
                       stack_map_entry.bit_tag,
                       stack_map_entry.offset_delta);
                print_verification_table(stack_map_entry.stack_items,
                                         stack_map_entry.stack_item_count);
                break;
            }
            case STACK_MAP_CHOP_FRAME: {
                printf(TOKEN_FMT_U8_U16 "(u8 ChopFrame, u16 OffsetDelta)\n",
                       stack_map_entry.bit_tag,
                       stack_map_entry.offset_delta);
                break;
            }
            case STACK_MAP_SAME_FRAME_EXTENDED: {
                printf(TOKEN_FMT_U8_U16 "(u8 SameFrameExtended, "
                                        "u16 OffsetDelta)\n",
                       stack_map_entry.bit_tag,
                       stack_map_entry.offset_delta);
                break;
            }
            case STACK_MAP_APPEND_FRAME: {
                printf(TOKEN_FMT_U8_U16 "(u8 AppendFrame, u16 OffsetDelta)\n",
                       stack_map_entry.bit_tag,
                       stack_map_entry.offset_delta);
                print_verification_table(stack_map_entry.local_items,
                                         stack_map_entry.local_item_count);
                break;
            }
            case STACK_MAP_FULL_FRAME: {
                printf(TOKEN_FMT_U8_U16 "(u8 FullFrame, u16 OffsetDelta)\n",
                       stack_map_entry.bit_tag,
                       stack_map_entry.offset_delta);
                printf("  %-18hu(u16 LocalItemCount)\n",
                       stack_map_entry.local_item_count);
                print_verification_table(stack_map_entry.local_items,
                                         stack_map_entry.local_item_count);
                printf("  %-18hu(u16 StackItemCount)\n",
                       stack_map_entry.stack_item_count);
                print_verification_table(stack_map_entry.stack_items,
                                         stack_map_entry.stack_item_count);
                break;
            }
            }
        }
        break;
    }
    case ATTRIB_SOURCE_FILE: {
        printf("[ SourceFileAttribute ]\n");
        printf(TOKEN_FMT_U16 "(u16 SourceFileIndex)\n", attribute->u16);
        break;
    }
    case ATTRIB_NEST_MEMBER: {
        printf("[ NestMember ]\n");
        printf(TOKEN_FMT_U16 "(u16 NestMemberCount)\n",
               attribute->nest_member.count);
        printf("  [");
        for (u16 i = 0; i < attribute->nest_member.count; ++i) {
            printf(" %hu", attribute->nest_member.classes[i]);
        }
        printf(" ]\n");
        break;
    }
    case ATTRIB_INNER_CLASSES: {
        printf("[ InnerClasses ]\n");
        printf(TOKEN_FMT_U16 "(u16 InnerClassesCount)\n",
               attribute->inner_classes.count);
        for (u16 i = 0; i < attribute->inner_classes.count; ++i) {
            InnerClassEntry inner_class_entry =
                attribute->inner_classes.entries[i];
            printf("  %-4hu%-4hu%-4hu%-6hu"
                   "(u16 InnerClassInfoIndex, u16 OuterClassInfoIndex,\n"
                   "                     "
                   "u16 InnerNameIndex, u16 InnerClassAccessFlags)\n",
                   inner_class_entry.inner_class_info_index,
                   inner_class_entry.outer_class_info_index,
                   inner_class_entry.inner_name_index,
                   inner_class_entry.inner_class_access_flags);
        }
    }
    }
}

void print_tokens(Memory* memory) {
    Token* tokens = memory->tokens;
    for (u32 i = 0; i < memory->token_index; ++i) {
        Token token = tokens[i];
        switch (token.tag) {
        case MAGIC: {
            printf("  0x%-16X(u32 Magic)\n\n", token.u32);
            break;
        }
        case MINOR_VERSION: {
            printf(TOKEN_FMT_U16 "(u16 MinorVersion)\n", token.u16);
            break;
        }
        case MAJOR_VERSION: {
            printf(TOKEN_FMT_U16 "(u16 MajorVersion)\n\n", token.u16);
            break;
        }
        case CONSTANT_POOL_COUNT: {
            printf(TOKEN_FMT_U16 "(u16 ConstantPoolCount)\n\n", token.u16);
            break;
        }
        case CONSTANT: {
            switch (token.constant.tag) {
            case CONSTANT_TAG_UTF8: {
                printf(CONSTANT_FMT_U8_U16_STRING CONSTANT_FMT_INDEX
                       "(u8 Constant.Utf8, u16 Length, u8*%hu String)\n",
                       (u8)token.constant.tag,
                       token.constant.utf8.size,
                       token.constant.utf8.string,
                       token.constant.index,
                       token.constant.utf8.size);
                break;
            }
            case CONSTANT_TAG_CLASS: {
                printf(CONSTANT_FMT_U8_U16 CONSTANT_FMT_INDEX
                       "(u8 Constant.Class, "
                       "u16 NameIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.class_.name_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_STRING: {
                printf(CONSTANT_FMT_U8_U16 CONSTANT_FMT_INDEX
                       "(u8 Constant.String, "
                       "u16 StringIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.string.string_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_FIELD_REF: {
                printf(CONSTANT_FMT_U8_U16_U16 CONSTANT_FMT_INDEX
                       "(u8 Constant.FieldRef, u16 ClassIndex, "
                       "u16 NameAndTypeIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.ref.class_index,
                       token.constant.ref.name_and_type_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_METHOD_REF: {
                printf(CONSTANT_FMT_U8_U16_U16 CONSTANT_FMT_INDEX
                       "(u8 Constant.MethodRef, u16 "
                       "ClassIndex,\n" CONSTANT_TAG_PAD
                       "u16 NameAndTypeIndex)\n",
                       (u8)token.constant.tag,
                       token.constant.ref.class_index,
                       token.constant.ref.name_and_type_index,
                       token.constant.index);
                break;
            }
            case CONSTANT_TAG_NAME_AND_TYPE: {
                printf(CONSTANT_FMT_U8_U16_U16 CONSTANT_FMT_INDEX
                       "(u8 Constant.NameAndType, u16 "
                       "NameIndex,\n" CONSTANT_TAG_PAD
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
            printf(TOKEN_FMT_U16 "(u16 ThisClass)\n", token.u16);
            break;
        }
        case SUPER_CLASS: {
            printf(TOKEN_FMT_U16 "(u16 SuperClass)\n", token.u16);
            break;
        }
        case INTERFACE_COUNT: {
            printf("\n" TOKEN_FMT_U16 "(u16 InterfaceCount)\n", token.u16);
            break;
        }
        case FIELD_COUNT: {
            printf("\n" TOKEN_FMT_U16 "(u16 FieldCount)\n", token.u16);
            break;
        }
        case METHOD_COUNT: {
            printf("\n" TOKEN_FMT_U16 "(u16 MethodCount)\n", token.u16);
            break;
        }
        case METHOD: {
            printf("\n  %-18hu(u16 MethodAccessFlags) [",
                   token.method.access_flags);
            for (u16 j = 0; j < 16; ++j) {
                MethodAccessFlag method_access_flag =
                    (MethodAccessFlag)((1 << j) & token.method.access_flags);
                switch (method_access_flag) {
                case METHOD_ACC_PUBLIC: {
                    printf(" ACC_PUBLIC");
                    break;
                }
                case METHOD_ACC_PRIVATE: {
                    printf(" ACC_PRIVATE");
                    break;
                }
                case METHOD_ACC_PROTECTED: {
                    printf(" ACC_PROTECTED");
                    break;
                }
                case METHOD_ACC_STATIC: {
                    printf(" ACC_STATIC");
                    break;
                }
                case METHOD_ACC_FINAL: {
                    printf(" ACC_FINAL");
                    break;
                }
                case METHOD_ACC_SYNCHRONIZED: {
                    printf(" ACC_SYNCHRONIZED");
                    break;
                }
                case METHOD_ACC_BRIDGE: {
                    printf(" ACC_BRIDGE");
                    break;
                }
                case METHOD_ACC_VARARGS: {
                    printf(" ACC_VARARGS");
                    break;
                }
                case METHOD_ACC_NATIVE: {
                    printf(" ACC_NATIVE");
                    break;
                }
                case METHOD_ACC_ABSTRACT: {
                    printf(" ACC_ABSTRACT");
                    break;
                }
                case METHOD_ACC_STRICT: {
                    printf(" ACC_STRICT");
                    break;
                }
                case METHOD_ACC_SYNTHETIC: {
                    printf(" ACC_SYNTHETIC");
                    break;
                }
                }
            }
            printf(" ]\n\n");
            printf("  %-4hu%-4hu%-10hu"
                   "(u16 MethodNameIndex, u16 MethodDescriptorIndex,\n"
                   "                     "
                   "u16 MethodAttributeCount)"
                   "\n",
                   token.method.name_index,
                   token.method.descriptor_index,
                   token.method.attribute_count);
            Attribute* attribute = token.method.attributes;
            for (u16 j = 0; j < token.method.attribute_count; ++j) {
                if (attribute != NULL) {
                    print_attribute(attribute);
                    attribute = attribute->next_attribute;
                }
            }
            break;
        }
        case ATTRIBUTE_COUNT: {
            printf("\n" TOKEN_FMT_U16 "(u16 AttributeCount)\n", token.u16);
            break;
        }
        case ATTRIBUTE: {
            print_attribute(token.attribute);
            break;
        }
        }
    }
}

#endif
