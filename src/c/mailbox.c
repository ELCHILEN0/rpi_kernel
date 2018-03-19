#include "include/mailbox.h"
#include "include/peripheral.h"

mailbox_t *mailbox0 = (mailbox_t *) MAILBOX_BASE;
core_mailbox_t *core_mailbox = (core_mailbox_t *) 0x40000050;

/*
 * With the exception of the property tags mailbox channel, when passing memory addresses
 * as the data part of a mailbox message, the addresses should be bus addresses as seen from the VC.
 * These vary depending on whether the L2 cache is enabled. If it is, physical memory is mapped to 
 * start at 0x40000000 by the VC MMU; if L2 caching is disabled, physical memory is mapped to 
 * start at 0xC0000000 by the VC MMU. Returned addresses (both those returned in the data part of the 
 * mailbox response and any written into the buffer you passed) will also be as mapped by the VC MMU. 
 * 
 * In the exceptional case when you are using the property tags mailbox channel you should send 
 * and receive physical addresses (the same as you'd see from the ARM before enabling the MMU).
 */

void mailbox_write(mailbox_t *mailbox, mailbox0_channel_t channel, uint32_t msg) {
    while (mailbox->status & MB_STATUS_FULL) { }
    mailbox->write = VC_MMU_BASE | (msg & 0xfffffff0) | (channel & 0xf);
}

uint32_t mailbox_read_beta(mailbox_t *mailbox, mailbox0_channel_t channel) {
    while (true) {
        while (mailbox->status & MB_STATUS_EMPTY) { }

        uint32_t data = mailbox->read;
        uint8_t read_channel = (uint8_t) (data & 0xf);
        data >>= 4;

        if (read_channel == channel)
            return data;
    }
}


void core_mailbox_interrupt_routing( uint8_t core_id, core_mailbox_interrupt_t type ) {
    core_mailbox->interrupt_routing[core_id] = type;
}
