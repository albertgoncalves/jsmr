#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef FILE File;

typedef uint16_t u16;
typedef uint32_t u32;

typedef int32_t i32;

#define COUNT_CHARS  512
#define COUNT_BUFFER 24
#define COUNT_TOKENS 128

typedef enum {
    TOKEN_MAJOR_VERSION,
    TOKEN_MINOR_VERSION,
    TOKEN_NUMBER,
} TokenTag;

typedef struct {
    union {
        u32 number;
    };
    TokenTag tag;
} Token;

typedef struct {
    u32   file_size;
    char  chars[COUNT_CHARS];
    char  buffer[COUNT_BUFFER];
    u32   token_index;
    Token tokens[COUNT_TOKENS];
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
    if (COUNT_TOKENS <= memory->token_index) {
        fprintf(stderr, "[ERROR] Unable to allocate new token\n");
        exit(EXIT_FAILURE);
    }
    return &memory->tokens[memory->token_index++];
}

static u32 get_decimal(const char* decimal) {
    u32 result = 0;
    while (*decimal) {
        u32 digit = (u32)(*(decimal++));
        u32 value = 0;
        if (('0' <= digit) && (digit <= '9')) {
            value = digit - '0';
        } else {
            fprintf(stderr, "[ERROR] Unable to parse decimal\n");
            exit(EXIT_FAILURE);
        }
        result = (result * 10) | (value & 0xF);
    }
    return result;
}

static u32 get_hex(const char* hex) {
    u32 result = 0;
    while (*hex) {
        u32 digit = (u32)(*(hex++));
        u32 value = 0;
        if (('0' <= digit) && (digit <= '9')) {
            value = digit - '0';
        } else if (('a' <= digit) && (digit <= 'f')) {
            value = (digit - 'a') + 10;
        } else if (('A' <= digit) && (digit <= 'F')) {
            value = (digit - 'A') + 10;
        } else {
            fprintf(stderr, "[ERROR] Unable to parse hex\n");
            exit(EXIT_FAILURE);
        }
        result = (result << 4) | (value & 0xF);
    }
    return result;
}

static void set_tokens(Memory* memory) {
    const char* chars = memory->chars;
    u16         file_size = (u16)memory->file_size;
    u16         lines = 1;
    for (u16 i = 0; i < file_size; ++i) {
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
        default: {
            u16 j = (u16)(i + 1);
            for (; j < file_size; ++j) {
                char x = chars[j];
                if ((x == ' ') || (x == '\t') || (x == '\n')) {
                    break;
                }
            }
            char* buffer = memory->buffer;
            u16   k = 0;
            for (; i < j; ++i) {
                buffer[k++] = chars[i];
            }
            buffer[k] = '\0';
            if (('0' <= buffer[0]) && (buffer[0] <= '9')) {
                Token* token = alloc_token(memory);
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
                Token* token = alloc_token(memory);
                token->tag = TOKEN_MINOR_VERSION;
            } else if (!strcmp(buffer, "major_version")) {
                Token* token = alloc_token(memory);
                token->tag = TOKEN_MAJOR_VERSION;
            }
        }
        }
    }
}

static void print_tokens(Memory* memory) {
    for (u32 i = 0; i < memory->token_index; ++i) {
        Token token = memory->tokens[i];
        switch (token.tag) {
        case TOKEN_MAJOR_VERSION: {
            printf("TOKEN_MAJOR_VERSION\n");
            break;
        }
        case TOKEN_MINOR_VERSION: {
            printf("TOKEN_MINOR_VERSION\n");
            break;
        }
        case TOKEN_NUMBER: {
            printf("TOKEN_NUMBER %u\n", token.number);
            break;
        }
        }
    }
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
    print_tokens(memory);
    printf("\nmemory->file_size : %u\nDone!\n", memory->file_size);
    free(memory);
    return EXIT_SUCCESS;
}
