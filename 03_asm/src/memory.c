#ifndef __MEMORY_C__
#define __MEMORY_C__

#include "memory.h"

static const char* BUFFER_MINUS = "-";
static const char* BUFFER_LBRACE = "{";
static const char* BUFFER_RBRACE = "}";

void set_file_to_chars(Memory* memory, const char* filename) {
    File* file = fopen(filename, "r");
    if (file == NULL) {
        ERROR("Unable to open file");
    }
    fseek(file, 0, SEEK_END);
    u32 file_size = (u32)ftell(file);
    rewind(file);
    if (SIZE_FILE < file_size) {
        ERROR("File does not fit into memory");
    }
    if (fread(&memory->file, sizeof(char), file_size, file) != file_size) {
        ERROR("`fread` failed");
    }
    memory->file_size = file_size;
    fclose(file);
}

Token* alloc_token(Memory* memory) {
    if (COUNT_TOKENS <= memory->token_count) {
        ERROR("Unable to allocate new token");
    }
    return &memory->tokens[memory->token_count++];
}

char* alloc_buffer(Memory* memory, u32 size) {
    if (SIZE_BUFFER < (memory->buffer_size + size)) {
        ERROR("Unable to allocate new buffer");
    }
    char* buffer = &memory->buffer[memory->buffer_size];
    memory->buffer_size += size;
    return buffer;
}

Constant* alloc_constant(Memory* memory) {
    if (COUNT_CONSTANTS <= memory->constant_count) {
        ERROR("Unable to allocate new constant");
    }
    return &memory->constants[memory->constant_count++];
}

Method* alloc_method(Memory* memory) {
    if (COUNT_METHODS <= memory->method_count) {
        ERROR("Unable to allocate new method");
    }
    return &memory->methods[memory->method_count++];
}

Op* alloc_op(Memory* memory) {
    if (COUNT_OPS <= memory->op_count) {
        ERROR("Unable to allocate new op");
    }
    return &memory->ops[memory->op_count++];
}

void set_tokens(Memory* memory) {
    const char* file = memory->file;
    u32         file_size = (u32)memory->file_size;
    u32         lines = 1;
    for (u32 i = 0; i < file_size; ++i) {
        switch (file[i]) {
        case ' ':
        case '\t': {
            break;
        }
        case '\n': {
            ++lines;
            break;
        }
        case ';': {
            for (; i < file_size; ++i) {
                if (file[i] == '\n') {
                    ++lines;
                    break;
                }
            }
            break;
        }
        case '-': {
            Token* token = alloc_token(memory);
            token->tag = TOKEN_MINUS;
            token->buffer = BUFFER_MINUS;
            token->line = lines;
            break;
        }
        case '{': {
            Token* token = alloc_token(memory);
            token->tag = TOKEN_LBRACE;
            token->buffer = BUFFER_LBRACE;
            token->line = lines;
            break;
        }
        case '}': {
            Token* token = alloc_token(memory);
            token->tag = TOKEN_RBRACE;
            token->buffer = BUFFER_RBRACE;
            token->line = lines;
            break;
        }
        default: {
            Token* token = alloc_token(memory);
            token->line = lines;
            u32 j = i + 1;
            if (file[i] == '"') {
                for (; j < file_size; ++j) {
                    char x = file[j];
                    if (x == '"') {
                        ++j;
                        break;
                    } else if (x == '\n') {
                        ++lines;
                    }
                }
            } else {
                for (; j < file_size; ++j) {
                    char x = file[j];
                    if ((x == ' ') || (x == '\t') || (x == ';')) {
                        break;
                    } else if (x == '\n') {
                        ++lines;
                        break;
                    }
                }
            }
            char* buffer = alloc_buffer(memory, (j - i) + 1);
            token->buffer = buffer;
            u32 k = 0;
            while (i < j) {
                buffer[k++] = file[i++];
            }
            buffer[k] = '\0';
            if (('0' <= buffer[0]) && (buffer[0] <= '9')) {
                token->tag = TOKEN_NUMBER;
                if ((1 < k) && (buffer[1] == 'x')) {
                    if (k == 2) {
                        HEX_ERROR;
                    }
                    token->number = get_hex(&buffer[2]);
                } else {
                    token->number = get_decimal(buffer);
                }
            } else if (buffer[0] == '.') {
                token->tag = TOKEN_OP;
                buffer[0] = '\0';
                token->buffer = &buffer[1];
            } else if (is_quote(buffer, k)) {
                token->tag = TOKEN_QUOTE;
                buffer[0] = '\0';
                buffer[k - 1] = '\0';
                token->buffer = &buffer[1];
            } else if (get_eq(buffer, "access_flags")) {
                token->tag = TOKEN_ACCESS_FLAGS;
            } else if (get_eq(buffer, "class")) {
                token->tag = TOKEN_CLASS;
            } else if (get_eq(buffer, "code")) {
                token->tag = TOKEN_CODE;
            } else if (get_eq(buffer, "constants")) {
                token->tag = TOKEN_CONSTANTS;
            } else if (get_eq(buffer, "field_ref")) {
                token->tag = TOKEN_FIELD_REF;
            } else if (get_eq(buffer, "major_version")) {
                token->tag = TOKEN_MAJOR_VERSION;
            } else if (get_eq(buffer, "max_local")) {
                token->tag = TOKEN_MAX_LOCAL;
            } else if (get_eq(buffer, "max_stack")) {
                token->tag = TOKEN_MAX_STACK;
            } else if (get_eq(buffer, "method")) {
                token->tag = TOKEN_METHOD;
            } else if (get_eq(buffer, "method_ref")) {
                token->tag = TOKEN_METHOD_REF;
            } else if (get_eq(buffer, "minor_version")) {
                token->tag = TOKEN_MINOR_VERSION;
            } else if (get_eq(buffer, "name_and_type")) {
                token->tag = TOKEN_NAME_AND_TYPE;
            } else if (get_eq(buffer, "name_index")) {
                token->tag = TOKEN_NAME_INDEX;
            } else if (get_eq(buffer, "PUBLIC")) {
                token->tag = TOKEN_ACC_PUBLIC;
            } else if (get_eq(buffer, "STATIC")) {
                token->tag = TOKEN_ACC_STATIC;
            } else if (get_eq(buffer, "string")) {
                token->tag = TOKEN_STRING;
            } else if (get_eq(buffer, "SUPER")) {
                token->tag = TOKEN_ACC_SUPER;
            } else if (get_eq(buffer, "super_class")) {
                token->tag = TOKEN_SUPER_CLASS;
            } else if (get_eq(buffer, "this_class")) {
                token->tag = TOKEN_THIS_CLASS;
            } else if (get_eq(buffer, "type_index")) {
                token->tag = TOKEN_TYPE_INDEX;
            } else {
                token->tag = TOKEN_UNKNOWN;
            }
        }
        }
    }
}

Token pop_token(Memory* memory) {
    if (COUNT_TOKENS <= memory->token_index) {
        ERROR("Unable to pop token");
    }
    return memory->tokens[memory->token_index++];
}

TokenTag peek_token_tag(Memory* memory) {
    return memory->tokens[memory->token_index].tag;
}

u32 get_unsigned(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_NUMBER) {
        UNEXPECTED_TOKEN(token.buffer, token.line);
    }
    return token.number;
}

i32 get_signed(Memory* memory) {
    Token token = pop_token(memory);
    Bool  negate = FALSE;
    if (token.tag == TOKEN_MINUS) {
        negate = TRUE;
        token = pop_token(memory);
    }
    if (token.tag != TOKEN_NUMBER) {
        UNEXPECTED_TOKEN(token.buffer, token.line);
    }
    if (negate) {
        return (i32)token.number * -1;
    }
    return (i32)token.number;
}

u32 pop_number(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_NUMBER) {
        UNEXPECTED_TOKEN(token.buffer, token.line);
    }
    return token.number;
}

void set_constants(Memory* memory) {
    EXPECTED_TOKEN(TOKEN_CONSTANTS, memory);
    EXPECTED_TOKEN(TOKEN_LBRACE, memory);
    memory->program.constants = NULL;
    for (;;) {
        Token token = pop_token(memory);
        if (token.tag == TOKEN_RBRACE) {
            break;
        }
        Constant* constant = alloc_constant(memory);
        if (memory->program.constants == NULL) {
            memory->program.constants = constant;
        }
        if (token.tag == TOKEN_CLASS) {
            constant->tag = CONST_CLASS;
            constant->name_index = (u16)pop_number(memory);
        } else if (token.tag == TOKEN_FIELD_REF) {
            constant->tag = CONST_FIELD_REF;
            constant->ref.class_index = (u16)pop_number(memory);
            constant->ref.name_and_type_index = (u16)pop_number(memory);
        } else if (token.tag == TOKEN_METHOD_REF) {
            constant->tag = CONST_METHOD_REF;
            constant->ref.class_index = (u16)pop_number(memory);
            constant->ref.name_and_type_index = (u16)pop_number(memory);
        } else if (token.tag == TOKEN_NAME_AND_TYPE) {
            constant->tag = CONST_NAME_AND_TYPE;
            constant->name_and_type.name_index = (u16)pop_number(memory);
            constant->name_and_type.type_index = (u16)pop_number(memory);
        } else if (token.tag == TOKEN_STRING) {
            constant->tag = CONST_STRING;
            constant->string_index = (u16)pop_number(memory);
        } else if (token.tag == TOKEN_QUOTE) {
            constant->tag = CONST_UTF8;
            constant->string = token.buffer;
        } else {
            UNEXPECTED_TOKEN(token.buffer, token.line);
        }
    }
    memory->program.constant_count = (u16)(memory->constant_count + 1);
}

void set_access_flags(Memory* memory) {
    EXPECTED_TOKEN(TOKEN_ACCESS_FLAGS, memory);
    EXPECTED_TOKEN(TOKEN_LBRACE, memory);
    for (;;) {
        Token token = pop_token(memory);
        if (token.tag == TOKEN_RBRACE) {
            break;
        } else if (token.tag == TOKEN_ACC_PUBLIC) {
            memory->program.access_flags |= 0x0001;
        } else if (token.tag == TOKEN_ACC_FINAL) {
            memory->program.access_flags |= 0x0010;
        } else if (token.tag == TOKEN_ACC_SUPER) {
            memory->program.access_flags |= 0x0020;
        } else if (token.tag == TOKEN_ACC_INTERFACE) {
            memory->program.access_flags |= 0x0200;
        } else if (token.tag == TOKEN_ACC_ABSTRACT) {
            memory->program.access_flags |= 0x0400;
        } else if (token.tag == TOKEN_ACC_SYNTHETIC) {
            memory->program.access_flags |= 0x1000;
        } else if (token.tag == TOKEN_ACC_ANNOTATION) {
            memory->program.access_flags |= 0x2000;
        } else if (token.tag == TOKEN_ACC_ENUM) {
            memory->program.access_flags |= 0x4000;
        } else if (token.tag == TOKEN_ACC_MODULE) {
            memory->program.access_flags |= 0x8000;
        } else {
            UNEXPECTED_TOKEN(token.buffer, token.line);
        }
    }
}

void set_interfaces(Memory* memory) {
    while (peek_token_tag(memory) == TOKEN_INTERFACE) {
        NOT_IMPLEMENTED
    }
}

void set_fields(Memory* memory) {
    while (peek_token_tag(memory) == TOKEN_INTERFACE) {
        NOT_IMPLEMENTED
    }
}

void set_method_access_flags(Memory* memory, Method* method) {
    EXPECTED_TOKEN(TOKEN_ACCESS_FLAGS, memory);
    EXPECTED_TOKEN(TOKEN_LBRACE, memory);
    for (;;) {
        Token token = pop_token(memory);
        if (token.tag == TOKEN_RBRACE) {
            break;
        } else if (token.tag == TOKEN_ACC_PUBLIC) {
            method->access_flags |= 0x0001;
        } else if (token.tag == TOKEN_ACC_STATIC) {
            method->access_flags |= 0x0008;
        } else {
            UNEXPECTED_TOKEN(token.buffer, token.line);
        }
    }
}

void set_method_code(Memory* memory, Method* method) {
    EXPECTED_TOKEN(TOKEN_CODE, memory);
    EXPECTED_TOKEN(TOKEN_LBRACE, memory);
    EXPECTED_TOKEN(TOKEN_MAX_STACK, memory);
    method->code.max_stack = (u16)get_unsigned(memory);
    EXPECTED_TOKEN(TOKEN_MAX_LOCAL, memory);
    method->code.max_local = (u16)get_unsigned(memory);
    EXPECTED_TOKEN(TOKEN_LBRACE, memory);
    method->code.ops = &memory->ops[memory->op_count];
    for (;;) {
        Token token = pop_token(memory);
        if (token.tag == TOKEN_OP) {
            ++method->code.op_count;
            Op* op = alloc_op(memory);
            if (get_eq(token.buffer, "iconst_0")) {
                op->tag = OP_ICONST_0;
            } else if (get_eq(token.buffer, "iconst_1")) {
                op->tag = OP_ICONST_1;
            } else if (get_eq(token.buffer, "iconst_2")) {
                op->tag = OP_ICONST_2;
            } else if (get_eq(token.buffer, "bipush")) {
                op->tag = OP_BIPUSH;
                op->i8 = (i8)get_signed(memory);
            } else if (get_eq(token.buffer, "ldc")) {
                op->tag = OP_LDC;
                op->u8 = (u8)get_unsigned(memory);
            } else if (get_eq(token.buffer, "iload")) {
                op->tag = OP_ILOAD;
                op->u8 = (u8)get_unsigned(memory);
            } else if (get_eq(token.buffer, "iload_0")) {
                op->tag = OP_ILOAD_0;
            } else if (get_eq(token.buffer, "iload_1")) {
                op->tag = OP_ILOAD_1;
            } else if (get_eq(token.buffer, "iload_2")) {
                op->tag = OP_ILOAD_2;
            } else if (get_eq(token.buffer, "iload_3")) {
                op->tag = OP_ILOAD_3;
            } else if (get_eq(token.buffer, "istore")) {
                op->tag = OP_ISTORE;
                op->u8 = (u8)get_unsigned(memory);
            } else if (get_eq(token.buffer, "istore_1")) {
                op->tag = OP_ISTORE_1;
            } else if (get_eq(token.buffer, "istore_2")) {
                op->tag = OP_ISTORE_2;
            } else if (get_eq(token.buffer, "istore_3")) {
                op->tag = OP_ISTORE_3;
            } else if (get_eq(token.buffer, "iadd")) {
                op->tag = OP_IADD;
            } else if (get_eq(token.buffer, "iinc")) {
                op->tag = OP_IINC;
                op->pair.u8 = (u8)get_unsigned(memory);
                op->pair.i8 = (i8)get_signed(memory);
            } else if (get_eq(token.buffer, "ifne")) {
                op->tag = OP_IFNE;
                op->i16 = (i16)get_signed(memory);
            } else if (get_eq(token.buffer, "if_icmpne")) {
                op->tag = OP_IF_ICMPNE;
                op->i16 = (i16)get_signed(memory);
            } else if (get_eq(token.buffer, "if_icmpge")) {
                op->tag = OP_IF_ICMPGE;
                op->i16 = (i16)get_signed(memory);
            } else if (get_eq(token.buffer, "goto")) {
                op->tag = OP_GOTO;
                op->i16 = (i16)get_signed(memory);
            } else if (get_eq(token.buffer, "ireturn")) {
                op->tag = OP_IRETURN;
            } else if (get_eq(token.buffer, "return")) {
                op->tag = OP_RETURN;
            } else if (get_eq(token.buffer, "getstatic")) {
                op->tag = OP_GETSTATIC;
                op->u16 = (u16)get_unsigned(memory);
            } else if (get_eq(token.buffer, "invokevirtual")) {
                op->tag = OP_INVOKEVIRTUAL;
                op->u16 = (u16)get_unsigned(memory);
            } else if (get_eq(token.buffer, "invokestatic")) {
                op->tag = OP_INVOKESTATIC;
                op->u16 = (u16)get_unsigned(memory);
            } else {
                UNEXPECTED_TOKEN(token.buffer, token.line);
            }
        } else if (token.tag == TOKEN_RBRACE) {
            break;
        } else {
            UNEXPECTED_TOKEN(token.buffer, token.line);
        }
    }
    EXPECTED_TOKEN(TOKEN_RBRACE, memory);
}

void set_methods(Memory* memory) {
    while (peek_token_tag(memory) == TOKEN_METHOD) {
        Method* method = alloc_method(memory);
        pop_token(memory);
        EXPECTED_TOKEN(TOKEN_LBRACE, memory);
        set_method_access_flags(memory, method);
        EXPECTED_TOKEN(TOKEN_NAME_INDEX, memory);
        method->name_index = (u16)get_unsigned(memory);
        EXPECTED_TOKEN(TOKEN_TYPE_INDEX, memory);
        method->type_index = (u16)get_unsigned(memory);
        set_method_code(memory, method);
        EXPECTED_TOKEN(TOKEN_RBRACE, memory);
    }
    memory->program.method_count = memory->method_count;
    memory->program.methods = memory->methods;
}

void set_attributes(Memory* memory) {
    while (peek_token_tag(memory) == TOKEN_ATTRIBUTE) {
        NOT_IMPLEMENTED
    }
}

void set_program(Memory* memory) {
    EXPECTED_TOKEN(TOKEN_MAJOR_VERSION, memory);
    memory->program.major_version = (u16)get_unsigned(memory);
    EXPECTED_TOKEN(TOKEN_MINOR_VERSION, memory);
    memory->program.minor_version = (u16)get_unsigned(memory);
    set_constants(memory);
    set_access_flags(memory);
    EXPECTED_TOKEN(TOKEN_THIS_CLASS, memory);
    memory->program.this_class = (u16)get_unsigned(memory);
    EXPECTED_TOKEN(TOKEN_SUPER_CLASS, memory);
    memory->program.super_class = (u16)get_unsigned(memory);
    set_interfaces(memory);
    set_fields(memory);
    set_methods(memory);
    set_attributes(memory);
}

#endif
