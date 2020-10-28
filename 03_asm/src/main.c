#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;

i32 main(i32 n, const char** args) {
    if (n < 3) {
        fprintf(stderr, "[ERROR] Missing arguments\n");
        exit(EXIT_FAILURE);
    }
    printf("%s\n%s\n", args[1], args[2]);
    return EXIT_SUCCESS;
}
