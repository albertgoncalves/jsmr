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
} TokenTag;

typedef struct {
    union {
        u32 number;
    };
    u32      line;
    TokenTag tag;
} Token;

typedef struct {
    u16 major_version;
    u16 minor_version;
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

static void set_file_to_chars(Memory* memory, const char* filename) {
    File* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "[ERROR] Unable to open file\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    u32 file_size = (u32)ftell(file);
    rewind(file);
    if (COUNT_CHARS < file_size) {
        fprintf(stderr, "[ERROR] File does not fit into memory\n");
        exit(EXIT_FAILURE);
    }
    if (fread(&memory->chars, sizeof(char), file_size, file) != file_size) {
        fprintf(stderr, "[ERROR] `fread` failed\n");
        exit(EXIT_FAILURE);
    }
    memory->file_size = file_size;
    fclose(file);
}

static Token* alloc_token(Memory* memory) {
    if (COUNT_TOKENS <= memory->token_count) {
        fprintf(stderr, "[ERROR] Unable to allocate new token\n");
        exit(EXIT_FAILURE);
    }
    return &memory->tokens[memory->token_count++];
}

static u32 get_decimal(const char* decimal) {
    u32 result = 0;
    while (*decimal) {
        u8  digit = (u8)(*(decimal++));
        u32 value = 0;
        if (('0' <= digit) && (digit <= '9')) {
            value = (u32)(digit - '0');
        } else {
            fprintf(stderr, "[ERROR] Unable to parse decimal\n");
            exit(EXIT_FAILURE);
        }
        result = (result * 10) + value;
    }
    return result;
}

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
            fprintf(stderr, "[ERROR] Unable to parse hex\n");
            exit(EXIT_FAILURE);
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
                fprintf(stderr, "[ERROR] String literal too large\n");
                exit(EXIT_FAILURE);
            }
            char* buffer = memory->buffer;
            u8    k = 0;
            for (; i < j; ++i) {
                buffer[k++] = chars[i];
            }
            buffer[k] = '\0';
            if (('0' <= buffer[0]) && (buffer[0] <= '9')) {
                token->tag = TOKEN_NUMBER;
                if ((1 < k) && (buffer[1] == 'x')) {
                    if (k == 2) {
                        fprintf(stderr, "[ERROR] Unable to parse hex\n");
                        exit(EXIT_FAILURE);
                    }
                    token->number = get_hex(&buffer[2]);
                } else {
                    token->number = get_decimal(buffer);
                }
            } else if (!strcmp(buffer, "minor_version")) {
                token->tag = TOKEN_MINOR_VERSION;
            } else if (!strcmp(buffer, "major_version")) {
                token->tag = TOKEN_MAJOR_VERSION;
            }
        }
        }
    }
}

#define TOKEN_FMT_LINE "ln. %-4u"

static void fprint_token(File* file, Token token) {
    switch (token.tag) {
    case TOKEN_MAJOR_VERSION: {
        fprintf(file, TOKEN_FMT_LINE "TOKEN_MAJOR_VERSION\n", token.line);
        break;
    }
    case TOKEN_MINOR_VERSION: {
        fprintf(file, TOKEN_FMT_LINE "TOKEN_MINOR_VERSION\n", token.line);
        break;
    }
    case TOKEN_NUMBER: {
        fprintf(file,
                TOKEN_FMT_LINE "TOKEN_NUMBER %u\n",
                token.line,
                token.number);
        break;
    }
    case TOKEN_MINUS: {
        fprintf(file, TOKEN_FMT_LINE "TOKEN_MINUS\n", token.line);
        break;
    }
    }
}

static Token pop_token(Memory* memory) {
    if (COUNT_TOKENS <= memory->token_index) {
        fprintf(stderr, "[ERROR] Unable to pop token\n");
        exit(EXIT_FAILURE);
    }
    return memory->tokens[memory->token_index++];
}

#define UNEXPECTED_TOKEN(function, token)                             \
    {                                                                 \
        fprintf(stderr, "[ERROR] `" function "` unexpected token\n"); \
        fprint_token(stderr, token);                                  \
        exit(EXIT_FAILURE);                                           \
    }

static u32 get_unsigned(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_NUMBER) {
        UNEXPECTED_TOKEN("get_unsigned", token);
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
//         UNEXPECTED_TOKEN("get_signed", token);
//     }
//     if (negate) {
//         return (i32)token.number * -1;
//     }
//     return (i32)token.number;
// }

static void set_major_version(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_MAJOR_VERSION) {
        UNEXPECTED_TOKEN("set_major_version", token);
    }
    memory->program.major_version = (u16)get_unsigned(memory);
}

static void set_minor_version(Memory* memory) {
    Token token = pop_token(memory);
    if (token.tag != TOKEN_MINOR_VERSION) {
        UNEXPECTED_TOKEN("set_minor_version", token);
    }
    memory->program.minor_version = (u16)get_unsigned(memory);
}

static void set_program(Memory* memory) {
    set_major_version(memory);
    set_minor_version(memory);
}

i32 main(i32 n, const char** args) {
    printf("sizeof(Token)  : %zu\n"
           "sizeof(Memory) : %zu\n"
           "\n",
           sizeof(Token),
           sizeof(Memory));
    if (n < 3) {
        fprintf(stderr, "[ERROR] Missing arguments\n");
        exit(EXIT_FAILURE);
    }
    Memory* memory = calloc(1, sizeof(Memory));
    set_file_to_chars(memory, args[1]);
    set_tokens(memory);
    set_program(memory);
    printf("\n"
           "memory->file_size             : %u\n"
           "memory->program.major_version : %hu\n"
           "memory->program.minor_version : %hu\n"
           "Done!\n",
           memory->file_size,
           memory->program.major_version,
           memory->program.minor_version);
    free(memory);
    return EXIT_SUCCESS;
}
