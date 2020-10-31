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

#endif
