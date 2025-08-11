#include <stdio.h>
#include <stdlib.h>

typedef struct Comp {
    void *foo;
    void *bar;
    int baz;
} Comp;

typedef struct Node {
    int id;
    void *foo;
    void *bar;
    void *baz;
} Node;


Node *do_stuff(Comp **arr, int size_of_arr) {
    printf("size_of_arr inside do_stuff(): %d \n", size_of_arr);
    return malloc(sizeof(Node *));
}

#define FOO(...) ({ \
    Comp *__arr[] = __VA_ARGS__; \
    do_stuff(__arr, sizeof(__arr) / sizeof(Comp *)); \
})

Comp *Comp_new() {
    return malloc(sizeof(Comp *));
}

int main() {
    Node *node = FOO({Comp_new(), Comp_new(), Comp_new(), Comp_new()});
}