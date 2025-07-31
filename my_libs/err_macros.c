#include <stdio.h>
#include <stdlib.h>

#define err_exit(...) ({printf(__VA_ARGS__); printf("\n"); getchar(); exit(EXIT_FAILURE); })

#define assert(cond) if (!(cond)) err_exit("Assertion failed: "#cond)

