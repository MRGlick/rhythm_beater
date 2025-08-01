
#include <stdio.h>

#define print_todo(...) ({printf("TODO: \n\t"__VA_ARGS__); printf("\n");})