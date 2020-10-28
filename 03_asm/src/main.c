#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef FILE File;

typedef uint16_t u16;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

#define COUNT_CHARS  512
#define COUNT_BUFFER 24
#define COUNT_TOKENS 128

typedef enum {
    TOKEN_NUMBER,
    TOKEN_MINOR_VERSION,
    TOKEN_MAJOR_VERSION,
} TokenTag;

typedef struct {
    union {
        u32 number;
    };
    TokenTag tag;
} Token;

typedef struct {
    usize file_size;
    char  chars[COUNT_CHARS];
    char  buffer[COUNT_BUFFER];
    usize token_index;
    Token tokens[COUNT_TOKENS];
} Memory;

static void set_file_to_chars(Memory* memory, const char* filename) {
    File* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "[ERROR] Unable to open file\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    usize file_size = (usize)ftell(file);
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
            u16 k = 0;
            for (; i < j; ++i) {
                buffer[k++] = chars[i];
            }
            buffer[k] = '\0';
            if (('0' <= buffer[0]) && (buffer[0] <= '9')) {
                Token* token = alloc_token(memory);
                token->tag = TOKEN_MINOR_VERSION;
                // if ((1 < k) && (buffer[1] == 'x')) {
                //     token->number = read_hex(...)
                // } else {
                //     token->number = read_decimal(...)
                // }
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

i32 main(i32 n, const char** args) {
    if (n < 3) {
        fprintf(stderr, "[ERROR] Missing arguments\n");
        exit(EXIT_FAILURE);
    }
    Memory* memory = calloc(1, sizeof(Memory));
    set_file_to_chars(memory, args[1]);
    set_tokens(memory);
    printf("\nmemory->file_size : %zu\nDone!\n", memory->file_size);
    free(memory);
    return EXIT_SUCCESS;
}
