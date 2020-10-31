#ifndef __TOKENS_C__
#define __TOKENS_C__

#include "tokens.h"

u32 get_decimal(const char* decimal) {
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

u32 get_hex(const char* hex) {
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

Bool is_quote(const char* buffer, u32 size) {
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
