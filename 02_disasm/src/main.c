#include "print.c"

i32 main(i32 n, const char** args) {
    printf("sizeof(Constant)         : %zu\n"
           "sizeof(Attribute)        : %zu\n"
           "sizeof(Code)             : %zu\n"
           "sizeof(VerificationType) : %zu\n"
           "sizeof(StackMapEntry)    : %zu\n"
           "sizeof(StackMapTable)    : %zu\n"
           "sizeof(Method)           : %zu\n"
           "sizeof(Token)            : %zu\n"
           "sizeof(Memory)           : %zu\n"
           "\n",
           sizeof(Constant),
           sizeof(Attribute),
           sizeof(Code),
           sizeof(VerificationType),
           sizeof(StackMapEntry),
           sizeof(StackMapTable),
           sizeof(Method),
           sizeof(Token),
           sizeof(Memory));
    if (n < 2) {
        fprintf(stderr, "[ERROR] No file provided\n");
        exit(EXIT_FAILURE);
    }
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        fprintf(stderr, "[ERROR] `calloc` failed\n");
        exit(EXIT_FAILURE);
    }
    set_file_to_bytes(memory, args[1]);
    set_tokens(memory);
    print_tokens(memory);
    printf("\n[INFO] %u bytes left!\n",
           memory->file_size - memory->byte_index);
    free(memory);
    return EXIT_SUCCESS;
}
