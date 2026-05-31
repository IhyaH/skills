#ifndef TJC_HAL_DRIVER_H
#define TJC_HAL_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tjc_ringbuf.h"

/*
 * This template intentionally covers only the return frames whose high-level
 * meaning is explicitly confirmed in the official documentation currently
 * bundled with this skill.
 */

#define TJC_RET_INVALID_CMD  0x00u
#define TJC_RET_OK           0x01u
#define TJC_RET_TOUCH_EVENT  0x65u
#define TJC_RET_PAGE_ID      0x66u
#define TJC_RET_TOUCH_POS    0x67u
#define TJC_RET_SLEEP_TOUCH  0x68u
#define TJC_RET_STRING       0x70u
#define TJC_RET_NUMBER       0x71u
#define TJC_RET_AUTO_SLEEP   0x86u
#define TJC_RET_AUTO_WAKE    0x87u
#define TJC_RET_STARTUP      0x88u
#define TJC_RET_SD_UPGRADE   0x89u
#define TJC_RET_BUF_OVF      0x24u
#define TJC_RET_PASSTHROUGH_DONE  0xFDu
#define TJC_RET_PASSTHROUGH_READY 0xFEu

typedef struct {
    tjc_ringbuf_t rx_rb;
} tjc_hal_driver_t;

typedef void (*tjc_send_bytes_fn)(const uint8_t *data, uint16_t len);
typedef void (*tjc_on_string_fn)(const uint8_t *data, uint16_t len);
typedef void (*tjc_on_number_fn)(int32_t value);
typedef void (*tjc_on_page_fn)(uint8_t page_id);
typedef void (*tjc_on_click_fn)(uint8_t page_id, uint8_t ctrl_id, uint8_t event);
typedef void (*tjc_on_touch_pos_fn)(uint16_t x, uint16_t y, uint8_t event, bool is_sleep);
typedef void (*tjc_on_status_fn)(uint8_t code);
typedef void (*tjc_on_raw_frame_fn)(const uint8_t *frame, uint16_t len);

typedef struct {
    tjc_send_bytes_fn send_bytes;
    tjc_on_string_fn on_string;
    tjc_on_number_fn on_number;
    tjc_on_page_fn on_page;
    tjc_on_click_fn on_click;
    tjc_on_touch_pos_fn on_touch_pos;
    tjc_on_status_fn on_status;
    tjc_on_raw_frame_fn on_unhandled;
} tjc_hal_hooks_t;

void tjc_hal_init(tjc_hal_driver_t *drv);
bool tjc_hal_rx_byte(tjc_hal_driver_t *drv, uint8_t byte);
bool tjc_hal_poll(tjc_hal_driver_t *drv, const tjc_hal_hooks_t *hooks);

bool tjc_hal_send_cmd(const tjc_hal_hooks_t *hooks, const char *ascii_cmd);
bool tjc_hal_set_text(const tjc_hal_hooks_t *hooks, const char *obj, const char *text);
bool tjc_hal_set_value(const tjc_hal_hooks_t *hooks, const char *obj, int32_t value);
bool tjc_hal_get_attr(const tjc_hal_hooks_t *hooks, const char *expr);
bool tjc_hal_page(const tjc_hal_hooks_t *hooks, const char *page_name);

#endif
