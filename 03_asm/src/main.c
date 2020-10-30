#include "prelude.h"

#include "program.h"
#include "tokens.h"

#define SIZE_FILE       1024
#define SIZE_BUFFER     512
#define COUNT_TOKENS    128
#define COUNT_CONSTANTS 64

typedef struct {
    u32      file_size;
    char     file[SIZE_FILE];
    u32      buffer_size;
    char     buffer[SIZE_BUFFER];
    u32      token_index;
    u32      token_count;
    Token    tokens[COUNT_TOKENS];
    u32      constant_count;
    Constant constants[COUNT_CONSTANTS];
    Program  program;
} Memory;

static const char* BUFFER_MINUS = "-";
static const char* BUFFER_LBRACE = "{";
static const char* BUFFER_RBRACE = "}";

static void set_file_to_chars(Memory* memory, const char* filename) {
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

static Token* alloc_token(Memory* memory) {
    if (COUNT_TOKENS <= memory->token_count) {
        ERROR("Unable to allocate new token");
    }
    return &memory->tokens[memory->token_count++];
}

static char* alloc_buffer(Memory* memory, u32 size) {
    if (SIZE_BUFFER < (memory->buffer_size + size)) {
        ERROR("Unable to allocate new buffer");
    }
    char* buffer = &memory->buffer[memory->buffer_size];
    memory->buffer_size += size;
    return buffer;
}

static Constant* alloc_constant(Memory* memory) {
    if (COUNT_CONSTANTS <= memory->constant_count) {
        ERROR("Unable to allocate new constant");
    }
    return &memory->constants[memory->constant_count++];
}

static void set_tokens(Memory* memory) {
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
            } else if (is_quote(buffer, k)) {
                token->tag = TOKEN_QUOTE;
                token->buffer = &buffer[1];
                buffer[0] = '\0';
                buffer[k - 1] = '\0';
            } else if (!strcmp(buffer, "access_flags")) {
                token->tag = TOKEN_ACCESS_FLAGS;
            } else if (!strcmp(buffer, "class")) {
                token->tag = TOKEN_CLASS;
            } else if (!strcmp(buffer, "constants")) {
                token->tag = TOKEN_CONSTANTS;
            } else if (!strcmp(buffer, "major_version")) {
                token->tag = TOKEN_MAJOR_VERSION;
            } else if (!strcmp(buffer, "minor_version")) {
                token->tag = TOKEN_MINOR_VERSION;
            } else if (!strcmp(buffer, "name_and_type")) {
                token->tag = TOKEN_NAME_AND_TYPE;
            } else if (!strcmp(buffer, "string")) {
                token->tag = TOKEN_STRING;
            } else if (!strcmp(buffer, "super")) {
                token->tag = TOKEN_SUPER;
            } else if (!strcmp(buffer, "super_class")) {
                token->tag = TOKEN_SUPER_CLASS;
            } else if (!strcmp(buffer, "this_class")) {
                token->tag = TOKEN_THIS_CLASS;
            } else {
                token->tag = TOKEN_IDENT;
            }
        }
        }
    }
}

static Token pop_token(Memory* memory) {
    if (COUNT_TOKENS <= memory->token_index) {
        ERROR("Unable to pop token");
    }
    return memory->tokens[memory->token_index++];
}

static u32 get_unsigned(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_NUMBER) {
        UNEXPECTED_TOKEN("get_unsigned", token.buffer, token.line);
    }
    return token.number;
}

// static i32 get_signed(Memory* memory) {
//     Token token = pop_token(memory);
//     Bool  negate = FALSE;
//     if (token.tag == TOKEN_MINUS) {
//         negate = TRUE;
//         token = pop_token(memory);
//     }
//     if (token.tag != TOKEN_NUMBER) {
//         UNEXPECTED_TOKEN("get_signed", token.buffer, token.line);
//     }
//     if (negate) {
//         return (i32)token.number * -1;
//     }
//     return (i32)token.number;
// }

static void set_major_version(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_MAJOR_VERSION) {
        UNEXPECTED_TOKEN("set_major_version", token.buffer, token.line);
    }
    memory->program.major_version = (u16)get_unsigned(memory);
}

static void set_minor_version(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_MINOR_VERSION) {
        UNEXPECTED_TOKEN("set_minor_version", token.buffer, token.line);
    }
    memory->program.minor_version = (u16)get_unsigned(memory);
}

static u32 pop_number(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_NUMBER) {
        UNEXPECTED_TOKEN("pop_number", token.buffer, token.line);
    }
    return token.number;
}

static void set_constants(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_CONSTANTS) {
        UNEXPECTED_TOKEN("set_constants", token.buffer, token.line);
    }
    token = pop_token(memory);
    if (token.tag != TOKEN_LBRACE) {
        UNEXPECTED_TOKEN("set_constants", token.buffer, token.line);
    }
    token = pop_token(memory);
    memory->program.constants = NULL;
    for (; token.tag != TOKEN_RBRACE; token = pop_token(memory)) {
        Constant* constant = alloc_constant(memory);
        if (memory->program.constants == NULL) {
            memory->program.constants = constant;
        }
        if (token.tag == TOKEN_CLASS) {
            constant->tag = CONST_CLASS;
            constant->name_index = (u16)pop_number(memory);
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
            UNEXPECTED_TOKEN("set_constants", token.buffer, token.line);
        }
    }
    memory->program.constant_count = (u16)(memory->constant_count + 1);
}

static void set_access_flags(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_ACCESS_FLAGS) {
        UNEXPECTED_TOKEN("set_access_flags", token.buffer, token.line);
    }
    token = pop_token(memory);
    if (token.tag != TOKEN_LBRACE) {
        UNEXPECTED_TOKEN("set_access_flags", token.buffer, token.line);
    };
    token = pop_token(memory);
    for (; token.tag != TOKEN_RBRACE; token = pop_token(memory)) {
        if (token.tag == TOKEN_PUBLIC) {
            memory->program.access_flags |= 0x0001;
        } else if (token.tag == TOKEN_FINAL) {
            memory->program.access_flags |= 0x0010;
        } else if (token.tag == TOKEN_SUPER) {
            memory->program.access_flags |= 0x0020;
        } else if (token.tag == TOKEN_INTERFACE) {
            memory->program.access_flags |= 0x0200;
        } else if (token.tag == TOKEN_ABSTRACT) {
            memory->program.access_flags |= 0x0400;
        } else if (token.tag == TOKEN_SYNTHETIC) {
            memory->program.access_flags |= 0x1000;
        } else if (token.tag == TOKEN_ANNOTATION) {
            memory->program.access_flags |= 0x2000;
        } else if (token.tag == TOKEN_ENUM) {
            memory->program.access_flags |= 0x4000;
        } else if (token.tag == TOKEN_MODULE) {
            memory->program.access_flags |= 0x8000;
        } else {
            UNEXPECTED_TOKEN("set_access_flags", token.buffer, token.line);
        }
    }
}

static void set_this_class(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_THIS_CLASS) {
        UNEXPECTED_TOKEN("set_this_class", token.buffer, token.line);
    }
    memory->program.this_class = (u16)get_unsigned(memory);
}

static void set_super_class(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_SUPER_CLASS) {
        UNEXPECTED_TOKEN("set_super_class", token.buffer, token.line);
    }
    memory->program.super_class = (u16)get_unsigned(memory);
}

static void set_program(Memory* memory) {
    set_major_version(memory);
    set_minor_version(memory);
    set_constants(memory);
    set_access_flags(memory);
    set_this_class(memory);
    set_super_class(memory);
}

i32 main(i32 n, const char** args) {
    printf("sizeof(Token)    : %zu\n"
           "sizeof(Constant) : %zu\n"
           "sizeof(Program)  : %zu\n"
           "sizeof(Memory)   : %zu\n"
           "\n",
           sizeof(Token),
           sizeof(Constant),
           sizeof(Program),
           sizeof(Memory));
    if (n < 3) {
        ERROR("Missing arguments");
    }
    Memory* memory = calloc(1, sizeof(Memory));
    set_file_to_chars(memory, args[1]);
    set_tokens(memory);
    set_program(memory);
    print_program(&memory->program);
    printf("\nDone!\n");
    free(memory);
    return EXIT_SUCCESS;
}
