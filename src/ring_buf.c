#include <serial_shell/ring_buf.h>
#include <stdbool.h>

void buf_push(ringbuf_t* rb, uint8_t c) {
    uint8_t next = (rb->head +1) % RING_BUFFER_SIZE;
    //ignore data if full
    if(next != rb->tail) {
        rb->buf[rb->head] = c;
        rb->head = next;
    }
}

bool buf_pop(ringbuf_t* rb, uint8_t* out) {
    if (rb->head == rb->tail) {return false;}
    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    return true;
}