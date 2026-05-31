#include "tjc_ringbuf.h"

static uint16_t tjc_ringbuf_advance(uint16_t index) {
    return (uint16_t)((index + 1u) % TJC_RINGBUF_CAPACITY);
}

void tjc_ringbuf_init(tjc_ringbuf_t *rb) {
    rb->head = 0u;
    rb->tail = 0u;
    rb->count = 0u;
}

bool tjc_ringbuf_push(tjc_ringbuf_t *rb, uint8_t byte) {
    if (rb->count >= TJC_RINGBUF_CAPACITY) {
        return false;
    }

    rb->data[rb->head] = byte;
    rb->head = tjc_ringbuf_advance(rb->head);
    rb->count++;
    return true;
}

bool tjc_ringbuf_pop(tjc_ringbuf_t *rb, uint8_t *byte) {
    if (rb->count == 0u) {
        return false;
    }

    *byte = rb->data[rb->tail];
    rb->tail = tjc_ringbuf_advance(rb->tail);
    rb->count--;
    return true;
}

uint16_t tjc_ringbuf_count(const tjc_ringbuf_t *rb) {
    return rb->count;
}

bool tjc_ringbuf_take_frame(tjc_ringbuf_t *rb, uint8_t *out_frame, uint16_t out_capacity, uint16_t *out_len) {
    uint16_t index;
    uint16_t i;
    uint16_t frame_len;

    if (rb->count < 3u) {
        return false;
    }

    index = rb->tail;
    for (i = 0u; i + 2u < rb->count; ++i) {
        uint8_t b0 = rb->data[index];
        uint8_t b1 = rb->data[(uint16_t)((index + 1u) % TJC_RINGBUF_CAPACITY)];
        uint8_t b2 = rb->data[(uint16_t)((index + 2u) % TJC_RINGBUF_CAPACITY)];
        if (b0 == 0xFFu && b1 == 0xFFu && b2 == 0xFFu) {
            frame_len = (uint16_t)(i + 3u);
            if (frame_len > out_capacity) {
                return false;
            }
            for (i = 0u; i < frame_len; ++i) {
                (void)tjc_ringbuf_pop(rb, &out_frame[i]);
            }
            *out_len = frame_len;
            return true;
        }
        index = tjc_ringbuf_advance(index);
    }

    return false;
}
