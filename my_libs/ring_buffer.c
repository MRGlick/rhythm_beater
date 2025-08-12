#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "move.c"
#include "dbg_macros.c"

typedef struct RingBuffer {

    int head, tail; // head = producer, tail = consumer because fuck convention, counted in bytes
    void *buffer;
    size_t capacity, item_size, count;

} RingBuffer;


RingBuffer *_RB_new(size_t item_size, size_t capacity);

#define RB_new(type, capacity) _RB_new(sizeof(type), capacity)

void _RB_free(RingBuffer *rb);

#define RB_free(rb) _RB_free(move(rb))

// sizeof(buf) < rb->item_size ==> Undefined Behaviour 
void _RB_pop(RingBuffer *rb, void *buf);

#define RB_pop(type, rb) ({type __dumdum; _RB_pop(rb, &__dumdum); __dumdum;})

void _RB_push(RingBuffer *rb, void *buf);

#define RB_push(type, rb, value) ({type __valval = (value); _RB_push(rb, &__valval);})

bool RB_is_empty(RingBuffer *rb);

void _RB_expand(RingBuffer *rb);

RingBuffer *_RB_new(size_t item_size, size_t capacity) {
    RingBuffer *rb = malloc(sizeof(RingBuffer));

    *rb = (RingBuffer) {
        .buffer = calloc(item_size, capacity),
        .capacity = capacity,
        .item_size = item_size
    };

    return rb;
}

void _RB_pop(RingBuffer *rb, void *buf) {
    
    assert(rb);
    assert(buf);
    if (RB_is_empty(rb)) err_exit(
        "Tried to pop from ring buffer, but there was nothing to pop!"
    );
    
    memcpy(buf, rb->buffer + rb->tail, rb->item_size);
    rb->tail = (rb->tail + rb->item_size) % (rb->capacity * rb->item_size);

    rb->count--;
}

void _RB_free(RingBuffer *rb) {

    assert(rb);

    free(rb->buffer);
    free(rb);
}

void _RB_push(RingBuffer *rb, void *buf) {
    assert(rb);
    assert(buf);

    memcpy(rb->buffer + rb->head, buf, rb->item_size);
    rb->head = (rb->head + rb->item_size) % (rb->capacity * rb->item_size);

    rb->count++;

    if (rb->count == rb->capacity) _RB_expand(rb);
}

bool RB_is_empty(RingBuffer *rb) {

    assert(rb);

    return rb->count == 0;
}

void _RB_expand(RingBuffer *rb) {
 
    assert(rb);

    size_t new_cap =  rb->capacity * 2;

    char *new_buf = calloc(rb->item_size, new_cap);

    int new_head = 0;

    int old_count = rb->count;

    while (!RB_is_empty(rb)) {
        _RB_pop(rb, new_buf + new_head);
        new_head += rb->item_size;
    }

    rb->buffer = new_buf;
    rb->capacity = new_cap;
    rb->head = new_head;
    rb->tail = 0;
    rb->count = old_count;

}