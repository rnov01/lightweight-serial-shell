#pragma once
#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 32

typedef struct ringbuf {
    volatile uint8_t buf[RING_BUFFER_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} ringbuf_t;

void buf_push(ringbuf_t* rb, uint8_t c);
bool buf_pop(ringbuf_t* rb, uint8_t* out);

extern ringbuf_t GLOBAL_BUFFER;