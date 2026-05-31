#ifndef TJC_RINGBUF_H
#define TJC_RINGBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TJC_RINGBUF_CAPACITY 1024u

typedef struct {
    uint8_t data[TJC_RINGBUF_CAPACITY];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} tjc_ringbuf_t;

void tjc_ringbuf_init(tjc_ringbuf_t *rb);
bool tjc_ringbuf_push(tjc_ringbuf_t *rb, uint8_t byte);
bool tjc_ringbuf_pop(tjc_ringbuf_t *rb, uint8_t *byte);
uint16_t tjc_ringbuf_count(const tjc_ringbuf_t *rb);

/*
 * Search for a complete frame terminated by FF FF FF.
 * On success, copies the frame bytes (including the terminator) into out_frame
 * and writes its length to out_len.
 */
bool tjc_ringbuf_take_frame(tjc_ringbuf_t *rb, uint8_t *out_frame, uint16_t out_capacity, uint16_t *out_len);

#endif
