#ifndef __PRINT_H__
#define __PRINT_H__

#include "memory.c"

#define TOKEN_FMT_U8     "  %-18hhu"
#define TOKEN_FMT_U16    "  %-18hu"
#define TOKEN_FMT_U8_U16 "  %-4hhu%-14hu"

#define OP_OFFSET    "%-14s"
#define OP_FMT_U8    OP_OFFSET "%hhu\n"
#define OP_FMT_U16   OP_OFFSET "%hu\n"
#define OP_FMT_I16   OP_OFFSET "%hd\n"
#define OP_FMT_U8_I8 OP_OFFSET "%-6hhu%hhd\n"

#define CONSTANT_TAG_PAD        "                          "
#define CONSTANT_FMT_U8         "  %-4hhu"
#define CONSTANT_FMT_U8_U16     CONSTANT_FMT_U8 "%-14hu"
#define CONSTANT_FMT_U8_U16_U16 CONSTANT_FMT_U8 "%-4hu%-10hu"
#define CONSTANT_FMT_U8_U16_STRING \
    CONSTANT_FMT_U8 "%-4hu\"%s\"\n                    "
#define CONSTANT_FMT_INDEX "#%-3hu "

void print_op_codes(const u8*, u32);
void print_verification_table(VerificationType*, u16);
void print_attribute(Attribute*);
void print_tokens(Memory*);

#endif
