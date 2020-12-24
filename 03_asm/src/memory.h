#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "program.c"
#include "tokens.c"

#define SIZE_FILE       4096
#define SIZE_BUFFER     2048
#define COUNT_TOKENS    256
#define COUNT_CONSTANTS 64
#define COUNT_METHODS   4
#define COUNT_OPS       64

typedef struct {
    Program  program;
    u32      file_size;
    char     file[SIZE_FILE];
    u32      buffer_size;
    char     buffer[SIZE_BUFFER];
    u32      token_index;
    u32      token_count;
    Token    tokens[COUNT_TOKENS];
    u16      constant_count;
    Constant constants[COUNT_CONSTANTS];
    u16      method_count;
    Method   methods[COUNT_METHODS];
    u16      op_count;
    Op       ops[COUNT_OPS];
} Memory;

void set_file_to_chars(Memory*, const char*);

Token*    alloc_token(Memory*);
char*     alloc_buffer(Memory*, u32);
Constant* alloc_constant(Memory*);
Method*   alloc_method(Memory*);
Op*       alloc_op(Memory*);

void     set_tokens(Memory*);
Token    pop_token(Memory*);
TokenTag peek_token_tag(Memory*);

u32 get_unsigned(Memory*);
i32 get_signed(Memory*);

u32 pop_number(Memory*);

void set_constants(Memory*);
void set_access_flags(Memory*);
void set_interfaces(Memory*);
void set_fields(Memory*);
void set_method_access_flags(Memory*, Method*);
void set_method_code(Memory*, Method*);
void set_methods(Memory*);
void set_attributes(Memory*);
void set_program(Memory*);

#endif
