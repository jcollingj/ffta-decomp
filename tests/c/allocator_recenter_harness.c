#include "allocator_recenter.h"
#include "allocator_packed.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define HOST_ASSERT(name, condition) typedef char name[(condition) ? 1 : -1]
HOST_ASSERT(owner_size, sizeof(AllocatorOwnerPacked) == 6);
HOST_ASSERT(owner_head, offsetof(AllocatorOwnerPacked, head) == 0);
HOST_ASSERT(owner_selection, offsetof(AllocatorOwnerPacked, selection) == 2);
HOST_ASSERT(owner_count, offsetof(AllocatorOwnerPacked, count) == 4);
HOST_ASSERT(node_size, sizeof(AllocatorNodePacked) == 12);
HOST_ASSERT(node_next, offsetof(AllocatorNodePacked, next) == 10);

#define WORDS 4096
#define NODE_BASE 8
#define NODE_STRIDE 4
#define MAX_NODES 700

static unsigned char actual[WORDS * 4];
static unsigned char expected[WORDS * 4];

static void put16(unsigned char *p, unsigned short value)
{
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8);
}

static void run_case(unsigned short count, unsigned int list_nodes,
                     int expected_node, int empty_head, int poison_next)
{
    unsigned int i;
    assert(list_nodes > 0 && list_nodes <= MAX_NODES);
    memset(actual, 0xA7, sizeof(actual));
    put16(actual, NODE_BASE);
    /* Plausible but wrong byte-offset decoy. */
    actual[6] = 0x39;
    actual[7] = 0x02;
    actual[2] = 0xD4;
    actual[3] = 0xB6;
    actual[4] = 0xE1;
    actual[5] = 0x5A;
    if (empty_head)
        put16(actual, 0);
    put16(actual + 4, count);

    for (i = 0; i < list_nodes; ++i) {
        unsigned int word_offset = NODE_BASE + i * NODE_STRIDE;
        unsigned char *node = actual + word_offset * 4;
        /* Decoys at nearby plausible offsets catch wrong field/base reads. */
        put16(node + 8, (unsigned short)(0x3000u + i));
        put16(node + 10, (unsigned short)(i == 0 && poison_next
                                               ? NODE_BASE + list_nodes * NODE_STRIDE
                                               : i + 1 < list_nodes
                                                     ? word_offset + NODE_STRIDE
                                                     : 0));
    }
    if (poison_next) {
        unsigned char *poison = actual + (NODE_BASE + list_nodes * NODE_STRIDE) * 4;
        put16(poison + 8, 0x6B31);
        put16(poison + 10, 0);
    }

    memcpy(expected, actual, sizeof(actual));
    put16(expected + 2, empty_head || expected_node < 0
                           ? 0
                           : (unsigned short)(NODE_BASE + expected_node * NODE_STRIDE));
    AllocatorRecenter(actual);
    if (memcmp(actual, expected, sizeof(actual)) != 0) {
        fprintf(stderr, "count=%u list=%u expected-node=%d empty=%d actual=%02x%02x expected=%02x%02x\n",
                count, list_nodes, expected_node, empty_head, actual[3], actual[2], expected[3], expected[2]);
        assert(0);
    }
}

int main(void)
{
    run_case(0, 1, 0, 0, 1); /* shifted count 0 skips poison successor */
    run_case(1, 1, 0, 0, 1);
    run_case(2, 1, 0, 0, 1);
    run_case(3, 1, 0, 0, 1); /* shifted count 1 also skips it */
    run_case(4, 1, 1, 0, 1); /* shifted count 2 follows exactly once */
    run_case(4, 1, -1, 0, 0); /* null successor is stored as zero */
    run_case(6, 3, 2, 0, 0);
    run_case(600, 301, 299, 0, 0);
    run_case(1398, 699, 698, 0, 0);
    run_case(0xFFFF, 1, 0, 1, 0);
    puts("allocator recenter behavior checks passed");
    return 0;
}
