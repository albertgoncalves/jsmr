#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef FILE File;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

#define SIZE 1024

typedef enum {
    MAGIC,
    MINOR_VERSION,
    MAJOR_VERSION,
    CONSTANT_POOL_SIZE,
} Tag;

typedef struct {
    union {
        u32 u32;
        u16 u16;
    };
    Tag tag;
} Token;

typedef struct {
    usize size;
    u8    bytes[SIZE];
    Token tokens[SIZE];
    usize byte_index;
    usize token_index;
} Memory;

#define OUT_OF_BOUNDS                       \
    {                                       \
        fprintf(stderr, "Out of bounds\n"); \
        exit(EXIT_FAILURE);                 \
    }

static u8 pop_u8(Memory* memory) {
    usize next_index = memory->byte_index + 1;
    if (memory->size < next_index) {
        OUT_OF_BOUNDS;
    }
    return memory->bytes[memory->byte_index++];
}

static u16 pop_u16(Memory* memory) {
    usize next_index = memory->byte_index + 2;
    if (memory->size < next_index) {
        OUT_OF_BOUNDS;
    }
    usize i = memory->byte_index;
    u16   bytes = (u16)((memory->bytes[i] << 8) | (memory->bytes[i + 1]));
    memory->byte_index = next_index;
    return bytes;
}

static u32 pop_u32(Memory* memory) {
    usize next_index = memory->byte_index + 4;
    if (memory->size < next_index) {
        OUT_OF_BOUNDS;
    }
    usize i = memory->byte_index;
    u32 bytes = (u32)((memory->bytes[i] << 24) | (memory->bytes[i + 1] << 16) |
                      (memory->bytes[i + 2] << 8) | memory->bytes[i + 3]);
    memory->byte_index = next_index;
    return bytes;
}

i32 main(i32 n, const char** args) {
    printf("sizeof(u8)     : %zu\n"
           "sizeof(Type)   : %zu\n"
           "sizeof(Token)  : %zu\n"
           "sizeof(Memory) : %zu\n"
           "\n",
           sizeof(u8),
           sizeof(Tag),
           sizeof(Token),
           sizeof(Memory));
    if (n < 2) {
        fprintf(stderr, "No file provided\n");
        exit(EXIT_FAILURE);
    }
    Memory* memory = malloc(sizeof(Memory));
    if (memory == NULL) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }
    {
        File* file = fopen(args[1], "rb");
        if (file == NULL) {
            fprintf(stderr, "Unable to open file\n");
            exit(EXIT_FAILURE);
        }
        fseek(file, 0, SEEK_END);
        usize size = (usize)ftell(file);
        rewind(file);
        if (SIZE < size) {
            fprintf(stderr, "File does not fit into memory\n");
            exit(EXIT_FAILURE);
        }
        if (fread(&memory->bytes, sizeof(u8), size, file) != size) {
            fprintf(stderr, "`fread` error\n");
            exit(EXIT_FAILURE);
        }
        memory->size = size;
        fclose(file);
    }
    Token* tokens = memory->tokens;
    memory->byte_index = 0;
    memory->token_index = 0;
    {
        u32 magic = pop_u32(memory);
        if (magic != 0xCAFEBABE) {
            fprintf(stderr, "Incorrect magic constant\n");
            exit(EXIT_FAILURE);
        }
        tokens[memory->token_index].tag = MAGIC;
        tokens[memory->token_index++].u32 = pop_u32(memory);
    }
    {
        tokens[memory->token_index].tag = MINOR_VERSION;
        tokens[memory->token_index++].u16 = pop_u16(memory);
        tokens[memory->token_index].tag = MAJOR_VERSION;
        tokens[memory->token_index++].u16 = pop_u16(memory);
    }
    {
        u16 constant_pool_size = pop_u16(memory);
        tokens[memory->token_index].tag = CONSTANT_POOL_SIZE;
        tokens[memory->token_index++].u16 = constant_pool_size;
        for (u16 i = 1; i < constant_pool_size; ++i) {
            u8 constant = pop_u8(memory);
            printf("! 0x%X\n", constant);
            break;
        }
    }
    for (usize i = 0; i < memory->token_index; ++i) {
        switch (tokens[i].tag) {
        case MAGIC: {
            printf("  (u32) 0x%X\n\n", tokens[i].u32);
            break;
        }
        case MINOR_VERSION: {
            printf("  (u16) Minor Version : %hu\n", tokens[i].u16);
            break;
        }
        case MAJOR_VERSION: {
            printf("  (u16) Major Version : %hu\n\n", tokens[i].u16);
            break;
        }
        case CONSTANT_POOL_SIZE: {
            printf("  (u16) Constant Pool Size : %hu\n", tokens[i].u16);
            break;
        }
        }
    }
    printf("\n[%zu bytes left!]\n", memory->size - memory->byte_index);
    free(memory);
    return EXIT_SUCCESS;
}
