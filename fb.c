#include "fb.h"

#include <stdlib.h>

#include "mailbox.h"

void *fb_create(uint32_t width, uint32_t height, uint32_t depth)
{
    PropertyMessageTag tags[4];
    tags[0].tag = FB_SET_PHYSICAL_SIZE;
    tags[0].value.fbScreenSize.width = width;
    tags[0].value.fbScreenSize.height = height;
    tags[1].tag = FB_SET_VIRTUAL_SIZE;
    tags[1].value.fbScreenSize.width = width;
    tags[1].value.fbScreenSize.height = height;
    tags[2].tag = FB_SET_DEPTH;
    tags[2].value.fbBitsPerPixel = depth;
    tags[3].tag = NULL_TAG;
    if (mailbox_send_messages(tags) != 0) {
        return NULL;
    }

    tags[0].tag = FB_ALLOCATE_TAG;
    tags[0].value.fbScreenSize.width = 0;
    tags[0].value.fbScreenSize.height = 0;
    tags[0].value.fbAllocateAlignment = 16;
    tags[1].tag = NULL_TAG;
    if (mailbox_send_messages(tags) != 0) {
        return NULL;
    }

    return (void *) tags[0].value.fbAllocateRes.base;
}