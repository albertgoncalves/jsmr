#ifndef __TOKENS_H__
#define __TOKENS_H__

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

#endif
