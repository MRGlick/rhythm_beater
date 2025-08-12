
#include "ring_buffer.c"
#include <stdio.h>

int main() {
    RingBuffer *rb = RB_new(int, 100);
    
    for (int i = 0; i < 100; i++) {
        RB_push(int, rb, i * 2);
    }

    while (!RB_is_empty(rb)) {
        printf("Popped value: %d \n", RB_pop(int, rb));
    }


    for (int i = 0; i < 200; i++) {
        RB_push(int, rb, i * 2);
    }

    while (!RB_is_empty(rb)) {
        printf("Popped value: %d \n", RB_pop(int, rb));
    }


    for (int i = 0; i < 300; i++) {
        RB_push(int, rb, i * 2);
    }

    while (!RB_is_empty(rb)) {
        printf("Popped value: %d \n", RB_pop(int, rb));
    }


    RB_free(rb);

    printf("Really finished! \n");
}