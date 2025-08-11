#include <stdio.h>
#include <stdlib.h>

#define print_dbg(...) ({printf("Debug:\n\t"__VA_ARGS__); printf("\n\t At line: %d \n", __LINE__);})

#define err_exit(...) ({fprintf(stderr, "ERROR!\n\t"__VA_ARGS__); printf("\n\t At line: %d \n", __LINE__); getchar(); exit(EXIT_FAILURE); })

#define assert(cond) if (!(cond)) err_exit("Assertion failed: "#cond)

