#include "memory.c"

i32 main(i32 n, const char** args) {
    printf("sizeof(Token)    : %zu\n"
           "sizeof(Constant) : %zu\n"
           "sizeof(Code)     : %zu\n"
           "sizeof(Op)       : %zu\n"
           "sizeof(Method)   : %zu\n"
           "sizeof(Program)  : %zu\n"
           "sizeof(Memory)   : %zu\n"
           "\n",
           sizeof(Token),
           sizeof(Constant),
           sizeof(Code),
           sizeof(Op),
           sizeof(Method),
           sizeof(Program),
           sizeof(Memory));
    if (n < 3) {
        ERROR("Missing arguments");
    }
    Memory* memory = calloc(1, sizeof(Memory));
    set_file_to_chars(memory, args[1]);
    set_tokens(memory);
    set_program(memory);
    print_program(&memory->program);
    serialize_program_to_file(&memory->program, args[2]);
    printf("Done!\n");
    free(memory);
    return EXIT_SUCCESS;
}
