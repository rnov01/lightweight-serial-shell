#pragma once
#include <stdint.h>

#define RING_BUFFER_SIZE 32

struct ringbuf {
    volatile uint8_t buf[RING_BUFFER_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
};

inline void buf_push(ringbuf* rb, uint8_t c) {
    uint8_t next = (rb->head +1) % RING_BUFFER_SIZE;
    //ignore data if full
    if(next != rb->tail) {
        rb->buf[rb->head] = c;
        rb->head = next;
    }
}

inline bool buf_pop(ringbuf* rb, uint8_t* out) {
    if (rb->head == rb->tail) {return false;}
    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    return true;
}

extern ringbuf GLOBAL_BUFFER;