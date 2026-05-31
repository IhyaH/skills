#include "tjc_hal_driver.h"

#include <stdio.h>
#include <string.h>

#define TJC_FRAME_END "\xFF\xFF\xFF"
#define TJC_FRAME_END_LEN 3u
#define TJC_TX_BUF_LEN 256u
#define TJC_RX_FRAME_MAX 512u

static bool tjc_hal_send_ascii(const tjc_hal_hooks_t *hooks, const char *ascii_cmd) {
    uint8_t tx[TJC_TX_BUF_LEN];
    size_t cmd_len;

    if (hooks == NULL || hooks->send_bytes == NULL || ascii_cmd == NULL) {
        return false;
    }

    cmd_len = strlen(ascii_cmd);
    if (cmd_len + TJC_FRAME_END_LEN > sizeof(tx)) {
        return false;
    }

    memcpy(tx, ascii_cmd, cmd_len);
    memcpy(tx + cmd_len, TJC_FRAME_END, TJC_FRAME_END_LEN);
    hooks->send_bytes(tx, (uint16_t)(cmd_len + TJC_FRAME_END_LEN));
    return true;
}

void tjc_hal_init(tjc_hal_driver_t *drv) {
    tjc_ringbuf_init(&drv->rx_rb);
}

bool tjc_hal_rx_byte(tjc_hal_driver_t *drv, uint8_t byte) {
    return tjc_ringbuf_push(&drv->rx_rb, byte);
}

static void tjc_hal_dispatch_frame(const uint8_t *frame, uint16_t len, const tjc_hal_hooks_t *hooks) {
    uint8_t code;
    int32_t value;

    if (len < 4u || hooks == NULL) {
        return;
    }

    code = frame[0];
    switch (code) {
        case TJC_RET_STRING:
            if (hooks->on_string != NULL && len >= 4u) {
                hooks->on_string(&frame[1], (uint16_t)(len - 4u));
            }
            break;

        case TJC_RET_NUMBER:
            if (hooks->on_number != NULL && len >= 8u) {
                value = (int32_t)(
                    ((uint32_t)frame[1]) |
                    ((uint32_t)frame[2] << 8) |
                    ((uint32_t)frame[3] << 16) |
                    ((uint32_t)frame[4] << 24)
                );
                hooks->on_number(value);
            }
            break;

        case TJC_RET_PAGE_ID:
            if (hooks->on_page != NULL && len >= 5u) {
                hooks->on_page(frame[1]);
            }
            break;

        case TJC_RET_TOUCH_EVENT:
            if (hooks->on_status != NULL) {
                hooks->on_status(code);
            }
            if (hooks->on_click != NULL && len >= 7u) {
                hooks->on_click(frame[1], frame[2], frame[3]);
            }
            break;

        case TJC_RET_INVALID_CMD:
        case TJC_RET_OK:
        case TJC_RET_TOUCH_POS:
        case TJC_RET_SLEEP_TOUCH:
            if (hooks->on_status != NULL) {
                hooks->on_status(code);
            }
            if (hooks->on_touch_pos != NULL && len >= 9u) {
                uint16_t x = ((uint16_t)frame[1] << 8) | frame[2];
                uint16_t y = ((uint16_t)frame[3] << 8) | frame[4];
                uint8_t ev = frame[5];
                hooks->on_touch_pos(x, y, ev, (code == TJC_RET_SLEEP_TOUCH));
            } else if (hooks->on_unhandled != NULL) {
                hooks->on_unhandled(frame, len);
            }
            break;

        case TJC_RET_AUTO_SLEEP:
        case TJC_RET_AUTO_WAKE:
        case TJC_RET_STARTUP:
        case TJC_RET_SD_UPGRADE:
        case TJC_RET_BUF_OVF:
        case TJC_RET_PASSTHROUGH_DONE:
        case TJC_RET_PASSTHROUGH_READY:
            if (hooks->on_status != NULL) {
                hooks->on_status(code);
            }
            break;

        default:
            if (hooks->on_unhandled != NULL) {
                hooks->on_unhandled(frame, len);
            }
            break;
    }
}

bool tjc_hal_poll(tjc_hal_driver_t *drv, const tjc_hal_hooks_t *hooks) {
    uint8_t frame[TJC_RX_FRAME_MAX];
    uint16_t len;

    if (!tjc_ringbuf_take_frame(&drv->rx_rb, frame, sizeof(frame), &len)) {
        return false;
    }

    tjc_hal_dispatch_frame(frame, len, hooks);
    return true;
}

bool tjc_hal_send_cmd(const tjc_hal_hooks_t *hooks, const char *ascii_cmd) {
    return tjc_hal_send_ascii(hooks, ascii_cmd);
}

bool tjc_hal_set_text(const tjc_hal_hooks_t *hooks, const char *obj, const char *text) {
    char cmd[TJC_TX_BUF_LEN - TJC_FRAME_END_LEN];
    if (obj == NULL || text == NULL) {
        return false;
    }
    if (snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", obj, text) < 0) {
        return false;
    }
    return tjc_hal_send_ascii(hooks, cmd);
}

bool tjc_hal_set_value(const tjc_hal_hooks_t *hooks, const char *obj, int32_t value) {
    char cmd[TJC_TX_BUF_LEN - TJC_FRAME_END_LEN];
    if (obj == NULL) {
        return false;
    }
    if (snprintf(cmd, sizeof(cmd), "%s.val=%ld", obj, (long)value) < 0) {
        return false;
    }
    return tjc_hal_send_ascii(hooks, cmd);
}

bool tjc_hal_get_attr(const tjc_hal_hooks_t *hooks, const char *expr) {
    char cmd[TJC_TX_BUF_LEN - TJC_FRAME_END_LEN];
    if (expr == NULL) {
        return false;
    }
    if (snprintf(cmd, sizeof(cmd), "get %s", expr) < 0) {
        return false;
    }
    return tjc_hal_send_ascii(hooks, cmd);
}

bool tjc_hal_page(const tjc_hal_hooks_t *hooks, const char *page_name) {
    char cmd[TJC_TX_BUF_LEN - TJC_FRAME_END_LEN];
    if (page_name == NULL) {
        return false;
    }
    if (snprintf(cmd, sizeof(cmd), "page %s", page_name) < 0) {
        return false;
    }
    return tjc_hal_send_ascii(hooks, cmd);
}
