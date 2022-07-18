#include "mailbox.h"

#define CLOCK_ARM 0x000000003

int clock_set_max_rate(void)
{
    PropertyMessageTag tags[2];
    tags[0].tag = CLOCK_GET_MAX_RATE;
    tags[0].value.clockId = CLOCK_ARM;
    tags[1].tag = NULL_TAG;
    if (mailbox_send_messages(tags) != 0) {
        return 1;
    }

    uint32_t rate = tags[0].value.clockRateRes.rate;

    tags[0].tag = CLOCK_SET_RATE;
    tags[0].value.clockRateSetInfo.clockId = CLOCK_ARM;
    tags[0].value.clockRateSetInfo.rate = rate;
    tags[0].value.clockRateSetInfo.skipTurbo = 0;
    tags[1].tag = NULL_TAG;
    if (mailbox_send_messages(tags) != 0) {
        return 1;
    }

    return 0;
}