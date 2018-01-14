#include <stdint.h>
#include <stdbool.h>

// https://github.com/raspberrypi/firmware/wiki/Mailboxes

// TODO: Determine why &message for parameter to write/read doesnt work but int32 array does.
// TODO: Fully implement the structs, (16? mailboxes on rpi3, 4/core)

typedef volatile struct {
    uint32_t id;
    uint32_t size;
    uint32_t code;
    uint32_t value[]; 
} tag_t;

typedef volatile struct {
    uint32_t size;
    uint32_t code;
    tag_t tag; // TODO: many tags...
    uint32_t end_tag;
} mailbox_message_t;

typedef volatile struct {
    uint32_t read;
    uint32_t reserved0[3];
    uint32_t peek;
    uint32_t sender;
    uint32_t status;
    uint32_t config;
    uint32_t write;
} mailbox_t;

typedef enum  {
    MB_STATUS_FULL     = 0x80000000,
    MB_STATUS_EMPTY    = 0x40000000,
    MB_STATUS_LEVEL    = 0x400000FF
} mailbox_status_t;

typedef enum {
    MB_REQUEST = 0x0,
    MB_RESPONSE_SUCCESS = 0x80000000,
    MB_RESPONSE_ERROR = 0x80000001
} mailbox_message_code_t;

typedef enum {
    MB0_POWER_MANAGEMENT,
    MB0_FRAMEBUFFER,
    MB0_VIRTUAL_UART,
    MB0_VCHIQ,
    MB0_LED,
    MB0_BUTTONS,
    MB0_TOUCH_SCREEN,
    MB0_RESERVED0,
    MB0_PROPERTY_TAGS_ARM_TO_VC,
    MB0_PROPERTY_TAGS_VC_TO_ARM
} mailbox0_channel_t;

extern mailbox_t *mailbox0;

extern void mailbox_write(mailbox_t *mailbox, mailbox0_channel_t channel, uint32_t msg);

// TODO: Verify it works...
extern uint32_t mailbox_read_beta(mailbox_t *mailbox, mailbox0_channel_t channel);