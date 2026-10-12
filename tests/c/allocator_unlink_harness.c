#include "allocator_unlink.h"
#include "allocator_recenter.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 2048
#define NODE_A_WORD 0x0120
#define NODE_B_WORD 0x0180
#define NODE_C_WORD 0x01F0
#define BYTE_AT(word) ((word) * 4)

static unsigned char actual[BUFFER_SIZE];
static unsigned char expected_at_call[BUFFER_SIZE];
static unsigned char final_expected[BUFFER_SIZE];
static unsigned char observed_at_call[BUFFER_SIZE];
static void *expected_owner;
static unsigned short expected_selected_offset;
static unsigned int call_count;
static unsigned int event_count;

static void put16(unsigned char *p, unsigned short value)
{
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8);
}

static void fixture(unsigned short head, unsigned short selected_offset,
                    unsigned short count)
{
    memset(actual, 0xA5, sizeof(actual));
    put16(actual, head);
    put16(actual + 2, selected_offset);
    put16(actual + 4, count);
}

/* Validate the full call-entry state, then model the helper's selected-offset
 * rewrite. The owner's +2 field is not assumed to mean a list tail. */
void AllocatorRecenter(void *owner)
{
    assert(owner == expected_owner);
    assert(event_count == 0);
    event_count = 1;
    ++call_count;
    memcpy(observed_at_call, actual, sizeof(actual));
    assert(memcmp(observed_at_call, expected_at_call, sizeof(actual)) == 0);
    put16((unsigned char *)owner + 2, expected_selected_offset);
}

static void prepare_three_nodes(void)
{
    fixture(NODE_A_WORD, NODE_C_WORD, 6);
    put16(actual + BYTE_AT(NODE_A_WORD) + 8, 0);
    put16(actual + BYTE_AT(NODE_A_WORD) + 10, NODE_B_WORD);
    put16(actual + BYTE_AT(NODE_B_WORD) + 8, NODE_A_WORD);
    put16(actual + BYTE_AT(NODE_B_WORD) + 10, NODE_C_WORD);
    put16(actual + BYTE_AT(NODE_C_WORD) + 8, NODE_B_WORD);
    put16(actual + BYTE_AT(NODE_C_WORD) + 10, 0);
}

static void run_case(unsigned int node, unsigned short selected_offset,
                     unsigned short new_head, unsigned short new_count,
                     unsigned short prev_next, unsigned short next_prev)
{
    unsigned char *block;

    prepare_three_nodes();
    block = actual + BYTE_AT(node);
    expected_owner = actual;
    expected_selected_offset = selected_offset;
    call_count = 0;
    event_count = 0;
    memcpy(expected_at_call, actual, sizeof(actual));

    /* Fixed per-node mutations are spelled out independently of the candidate. */
    if (node == NODE_A_WORD) {
        put16(expected_at_call, new_head);
        put16(expected_at_call + 4, new_count);
        put16(expected_at_call + BYTE_AT(NODE_B_WORD) + 8, 0);
        put16(expected_at_call + BYTE_AT(NODE_A_WORD) + 8, 0);
        put16(expected_at_call + BYTE_AT(NODE_A_WORD) + 10, 0);
    } else if (node == NODE_B_WORD) {
        put16(expected_at_call + BYTE_AT(NODE_A_WORD) + 10, prev_next);
        put16(expected_at_call + BYTE_AT(NODE_C_WORD) + 8, next_prev);
        put16(expected_at_call + 4, new_count);
        put16(expected_at_call + BYTE_AT(NODE_B_WORD) + 8, 0);
        put16(expected_at_call + BYTE_AT(NODE_B_WORD) + 10, 0);
    } else {
        put16(expected_at_call + BYTE_AT(NODE_B_WORD) + 10, 0);
        put16(expected_at_call + 4, new_count);
        put16(expected_at_call + BYTE_AT(NODE_C_WORD) + 8, 0);
        put16(expected_at_call + BYTE_AT(NODE_C_WORD) + 10, 0);
    }
    memcpy(final_expected, expected_at_call, sizeof(actual));
    put16(final_expected + 2, selected_offset);

    AllocatorUnlinkFreeBlock(actual, block);
    assert(call_count == 1);
    assert(event_count == 1);
    assert(memcmp(actual, final_expected, sizeof(actual)) == 0);
}

static void test_sole_node(void)
{
    fixture(NODE_A_WORD, NODE_A_WORD, 1);
    put16(actual + BYTE_AT(NODE_A_WORD) + 8, 0);
    put16(actual + BYTE_AT(NODE_A_WORD) + 10, 0);
    memcpy(expected_at_call, actual, sizeof(actual));
    put16(expected_at_call, 0);
    put16(expected_at_call + 4, 0);
    put16(expected_at_call + BYTE_AT(NODE_A_WORD) + 8, 0);
    put16(expected_at_call + BYTE_AT(NODE_A_WORD) + 10, 0);
    expected_owner = actual;
    expected_selected_offset = 0;
    call_count = 0;
    event_count = 0;
    memcpy(final_expected, expected_at_call, sizeof(actual));
    put16(final_expected + 2, 0);
    AllocatorUnlinkFreeBlock(actual, actual + BYTE_AT(NODE_A_WORD));
    assert(call_count == 1 && event_count == 1);
    assert(memcmp(actual, final_expected, sizeof(actual)) == 0);
}

static void test_zero_count_wrap(void)
{
    fixture(NODE_A_WORD, NODE_A_WORD, 0);
    put16(actual + BYTE_AT(NODE_A_WORD) + 8, 0);
    put16(actual + BYTE_AT(NODE_A_WORD) + 10, 0);
    memcpy(expected_at_call, actual, sizeof(actual));
    put16(expected_at_call, 0);
    put16(expected_at_call + 4, 0xFFFF);
    expected_owner = actual;
    expected_selected_offset = 0;
    call_count = 0;
    event_count = 0;
    memcpy(final_expected, expected_at_call, sizeof(actual));
    put16(final_expected + 2, 0);
    AllocatorUnlinkFreeBlock(actual, actual + BYTE_AT(NODE_A_WORD));
    assert(call_count == 1 && event_count == 1);
    assert(memcmp(actual, final_expected, sizeof(actual)) == 0);
}

int main(void)
{
    test_sole_node();
    test_zero_count_wrap();

    /* Head has no previous link; the next block becomes the head. */
    run_case(NODE_A_WORD, NODE_C_WORD, NODE_B_WORD, 5,
             NODE_C_WORD, 0);

    /* Middle reconnects both neighbors, preserving every unrelated byte. */
    run_case(NODE_B_WORD, NODE_C_WORD, NODE_A_WORD, 5,
             NODE_C_WORD, NODE_A_WORD);

    /* The last block has no successor; its previous link is disconnected. */
    run_case(NODE_C_WORD, NODE_B_WORD, NODE_A_WORD, 5,
             0, NODE_B_WORD);

    puts("allocator unlink state checks passed");
    return 0;
}
