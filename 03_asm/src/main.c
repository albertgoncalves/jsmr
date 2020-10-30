#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef FILE File;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int32_t i32;

#define COUNT_CHARS  512
#define COUNT_BUFFER 24
#define COUNT_TOKENS 128

// typedef enum {
//     FALSE = 0,
//     TRUE,
// } Bool;

typedef enum {
    TOKEN_MAJOR_VERSION,
    TOKEN_MINOR_VERSION,
    TOKEN_NUMBER,
    TOKEN_MINUS,
    TOKEN_UNKNOWN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_ACCESS_FLAGS,
    TOKEN_ACCESS_FLAG_PUBLIC,
    TOKEN_ACCESS_FLAG_FINAL,
    TOKEN_ACCESS_FLAG_SUPER,
    TOKEN_ACCESS_FLAG_INTERFACE,
    TOKEN_ACCESS_FLAG_ABSTRACT,
    TOKEN_ACCESS_FLAG_SYNTHETIC,
    TOKEN_ACCESS_FLAG_ANNOTATION,
    TOKEN_ACCESS_FLAG_ENUM,
    TOKEN_ACCESS_FLAG_MODULE,
} TokenTag;

typedef struct {
    const char* buffer;
    union {
        u32 number;
    };
    u32      line;
    TokenTag tag;
} Token;

typedef struct {
    u16 major_version;
    u16 minor_version;
    // constant_pool_count;
    // constants;
    u16 access_flags;
} Program;

typedef struct {
    u32     file_size;
    char    chars[COUNT_CHARS];
    char    buffer[COUNT_BUFFER];
    u32     token_index;
    u32     token_count;
    Token   tokens[COUNT_TOKENS];
    Program program;
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
    if (COUNT_CHARS < file_size) {
        ERROR("File does not fit into memory");
    }
    if (fread(&memory->chars, sizeof(char), file_size, file) != file_size) {
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

static void set_tokens(Memory* memory) {
    const char* chars = memory->chars;
    u32         file_size = (u32)memory->file_size;
    u32         lines = 1;
    for (u32 i = 0; i < file_size; ++i) {
        switch (chars[i]) {
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
                if (chars[i] == '\n') {
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
                char x = chars[j];
                if ((x == ' ') || (x == '\t')) {
                    break;
                } else if (x == '\n') {
                    ++lines;
                    break;
                }
            }
            if ((COUNT_BUFFER - 1) < (j - i)) {
                ERROR("String literal too large");
            }
            char* buffer = memory->buffer;
            token->buffer = buffer;
            u8 k = 0;
            for (; i < j; ++i) {
                buffer[k++] = chars[i];
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
            } else if (!strcmp(buffer, "minor_version")) {
                token->tag = TOKEN_MINOR_VERSION;
            } else if (!strcmp(buffer, "major_version")) {
                token->tag = TOKEN_MAJOR_VERSION;
            } else if (!strcmp(buffer, "access_flags")) {
                token->tag = TOKEN_ACCESS_FLAGS;
            } else if (!strcmp(buffer, "super")) {
                token->tag = TOKEN_ACCESS_FLAG_SUPER;
            } else {
                UNEXPECTED_TOKEN("set_tokens", buffer, lines);
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
        if (token.tag == TOKEN_ACCESS_FLAG_PUBLIC) {
            memory->program.access_flags |= 0x0001;
        } else if (token.tag == TOKEN_ACCESS_FLAG_FINAL) {
            memory->program.access_flags |= 0x0010;
        } else if (token.tag == TOKEN_ACCESS_FLAG_SUPER) {
            memory->program.access_flags |= 0x0020;
        } else if (token.tag == TOKEN_ACCESS_FLAG_INTERFACE) {
            memory->program.access_flags |= 0x0200;
        } else if (token.tag == TOKEN_ACCESS_FLAG_ABSTRACT) {
            memory->program.access_flags |= 0x0400;
        } else if (token.tag == TOKEN_ACCESS_FLAG_SYNTHETIC) {
            memory->program.access_flags |= 0x1000;
        } else if (token.tag == TOKEN_ACCESS_FLAG_ANNOTATION) {
            memory->program.access_flags |= 0x2000;
        } else if (token.tag == TOKEN_ACCESS_FLAG_ENUM) {
            memory->program.access_flags |= 0x4000;
        } else if (token.tag == TOKEN_ACCESS_FLAG_MODULE) {
            memory->program.access_flags |= 0x8000;
        } else {
            UNEXPECTED_TOKEN("set_access_flags", token.buffer, token.line);
        }
    }
}

static void set_program(Memory* memory) {
    set_major_version(memory);
    set_minor_version(memory);
    set_access_flags(memory);
}

i32 main(i32 n, const char** args) {
    printf("sizeof(Token)   : %zu\n"
           "sizeof(Program) : %zu\n"
           "sizeof(Memory)  : %zu\n"
           "\n",
           sizeof(Token),
           sizeof(Program),
           sizeof(Memory));
    if (n < 3) {
        ERROR("Missing arguments");
    }
    Memory* memory = calloc(1, sizeof(Memory));
    set_file_to_chars(memory, args[1]);
    set_tokens(memory);
    set_program(memory);
    printf("memory->file_size             : %u\n"
           "memory->program.major_version : %hu\n"
           "memory->program.minor_version : %hu\n"
           "memory->program.access_flags  : 0x%X\n"
           "Done!\n",
           memory->file_size,
           memory->program.major_version,
           memory->program.minor_version,
           memory->program.access_flags);
    free(memory);
    return EXIT_SUCCESS;
}
