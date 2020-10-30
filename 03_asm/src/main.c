#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef FILE File;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int32_t i32;

#define SIZE_FILE       512
#define SIZE_BUFFER     512
#define COUNT_TOKENS    128
#define COUNT_CONSTANTS 64

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

typedef enum {
    TOKEN_ABSTRACT,
    TOKEN_ACCESS_FLAGS,
    TOKEN_ANNOTATION,
    TOKEN_CLASS,
    TOKEN_CONSTANTS,
    TOKEN_ENUM,
    TOKEN_FINAL,
    TOKEN_IDENT,
    TOKEN_INTERFACE,
    TOKEN_LBRACE,
    TOKEN_MAJOR_VERSION,
    TOKEN_MINOR_VERSION,
    TOKEN_MINUS,
    TOKEN_MODULE,
    TOKEN_NUMBER,
    TOKEN_PUBLIC,
    TOKEN_QUOTE,
    TOKEN_RBRACE,
    TOKEN_STRING,
    TOKEN_SUPER,
    TOKEN_SYNTHETIC,
} TokenTag;

typedef struct {
    const char* buffer;
    u32         number;
    u32         line;
    TokenTag    tag;
} Token;

typedef enum {
    CONST_CLASS,
    CONST_UTF8,
} ConstantTag;

typedef struct {
    union {
        const char* string;
        u16         name_index;
    };
    ConstantTag tag;
} Constant;

typedef struct {
    u16             major_version;
    u16             minor_version;
    u16             constant_count;
    const Constant* constants;
    u16             access_flags;
} Program;

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

#define ERROR(message)                            \
    {                                             \
        fprintf(stderr, "[ERROR] " message "\n"); \
        exit(EXIT_FAILURE);                       \
    }

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
    if (SIZE_BUFFER <= (memory->buffer_size + size)) {
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

#define UNEXPECTED_TOKEN(function, buffer, line)                             \
    {                                                                        \
        fprintf(stderr,                                                      \
                "[ERROR] `" function "` unexpected token \"%s\" (ln. %u)\n", \
                buffer,                                                      \
                line);                                                       \
        exit(EXIT_FAILURE);                                                  \
    }

static u32 get_decimal(const char* decimal) {
    u32 result = 0;
    while (*decimal) {
        u8  digit = (u8)(*(decimal++));
        u32 value = 0;
        if (('0' <= digit) && (digit <= '9')) {
            value = (u32)(digit - '0');
        } else {
            ERROR("Unable to parse decimal");
        }
        result = (result * 10) + value;
    }
    return result;
}

#define HEX_ERROR ERROR("Unable to parse hex")

static u32 get_hex(const char* hex) {
    u32 result = 0;
    while (*hex) {
        u8  digit = (u8)(*(hex++));
        u32 value = 0;
        if (('0' <= digit) && (digit <= '9')) {
            value = (u32)(digit - '0');
        } else if (('a' <= digit) && (digit <= 'f')) {
            value = (u32)((digit - 'a') + 10);
        } else if (('A' <= digit) && (digit <= 'F')) {
            value = (u32)((digit - 'A') + 10);
        } else {
            HEX_ERROR;
        }
        result = (result << 4) + value;
    }
    return result;
}

static Bool is_quote(const char* buffer, u32 size) {
    if ((buffer[0] != '"') || (buffer[size - 1] != '"')) {
        return FALSE;
    }
    for (u32 i = 1; i < size - 1; ++i) {
        if (buffer[i] == '"') {
            return FALSE;
        }
    }
    return TRUE;
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
            for (; j < file_size; ++j) {
                char x = file[j];
                if ((x == ' ') || (x == '\t')) {
                    break;
                } else if (x == '\n') {
                    ++lines;
                    break;
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
                token->tag = TOKEN_STRING;
                token->buffer = &buffer[1];
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
            } else if (!strcmp(buffer, "super")) {
                token->tag = TOKEN_SUPER;
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
        if (token.tag == TOKEN_STRING) {
            constant->tag = CONST_UTF8;
            constant->string = token.buffer;
        } else if (token.tag == TOKEN_CLASS) {
            constant->tag = CONST_CLASS;
            token = pop_token(memory);
            if (token.tag != TOKEN_NUMBER) {
                UNEXPECTED_TOKEN("set_constants", token.buffer, token.line);
            }
            constant->name_index = (u16)token.number;
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

static void set_program(Memory* memory) {
    set_major_version(memory);
    set_minor_version(memory);
    set_constants(memory);
    set_access_flags(memory);
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
    printf("memory->file_size              : %u\n"
           "memory->program.major_version  : %hu\n"
           "memory->program.minor_version  : %hu\n"
           "memory->program.access_flags   : 0x%X\n"
           "memory->program.constant_count : %hu\n"
           "Done!\n",
           memory->file_size,
           memory->program.major_version,
           memory->program.minor_version,
           memory->program.access_flags,
           memory->program.constant_count);
    free(memory);
    return EXIT_SUCCESS;
}
