#ifndef __TOKENS_H__
#define __TOKENS_H__

typedef enum {
    TOKEN_ACC_ABSTRACT,
    TOKEN_ACC_ANNOTATION,
    TOKEN_ACC_ENUM,
    TOKEN_ACCESS_FLAGS,
    TOKEN_ACC_FINAL,
    TOKEN_ACC_INTERFACE,
    TOKEN_ACC_MODULE,
    TOKEN_ACC_PUBLIC,
    TOKEN_ACC_STATIC,
    TOKEN_ACC_SUPER,
    TOKEN_ACC_SYNTHETIC,
    TOKEN_ATTRIBUTE,
    TOKEN_CLASS,
    TOKEN_CODE,
    TOKEN_CONSTANTS,
    TOKEN_FIELD_REF,
    TOKEN_INTERFACE,
    TOKEN_LBRACE,
    TOKEN_MAJOR_VERSION,
    TOKEN_MAX_LOCAL,
    TOKEN_MAX_STACK,
    TOKEN_METHOD,
    TOKEN_METHOD_REF,
    TOKEN_MINOR_VERSION,
    TOKEN_MINUS,
    TOKEN_NAME_AND_TYPE,
    TOKEN_NAME_INDEX,
    TOKEN_NUMBER,
    TOKEN_OP,
    TOKEN_QUOTE,
    TOKEN_RBRACE,
    TOKEN_STRING,
    TOKEN_SUPER_CLASS,
    TOKEN_THIS_CLASS,
    TOKEN_TYPE_INDEX,
    TOKEN_UNKNOWN,
} TokenTag;

typedef struct {
    const char* buffer;
    u32         number;
    u32         line;
    TokenTag    tag;
} Token;

#define UNEXPECTED_TOKEN(buffer, line)                             \
    {                                                              \
        fprintf(stderr,                                            \
                "[ERROR] `%s` unexpected token \"%s\" (ln. %u)\n", \
                __func__,                                          \
                buffer,                                            \
                line);                                             \
        exit(EXIT_FAILURE);                                        \
    }

#define EXPECTED_TOKEN(token_tag, memory)               \
    {                                                   \
        Token token = pop_token(memory);                \
        if (token.tag != token_tag) {                   \
            UNEXPECTED_TOKEN(token.buffer, token.line); \
        }                                               \
    }

#define HEX_ERROR ERROR("Unable to parse hex")

u32  get_decimal(const char*);
u32  get_hex(const char*);
Bool is_quote(const char*, u32);

#endif
