#include "allocator_insert.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 8192
#define BYTE_AT(word) ((word) * 4)

static unsigned char actual[BUFFER_SIZE];
static unsigned char initial[BUFFER_SIZE];
static unsigned char expected_at_recenter[BUFFER_SIZE];
static unsigned char expected_final[BUFFER_SIZE];
static void *expected_owner;
static void *expected_block;
static void *expected_previous;
static void *expected_next;
static unsigned short expected_size;
static unsigned short recentered_offset;
static unsigned int call_order;

static void put16(unsigned char *p, unsigned short value)
{
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8);
}

static void begin_case(unsigned short block_word, unsigned short size,
                       unsigned short head, unsigned short count)
{
    memset(actual, 0xA5, sizeof(actual));
    put16(actual, head);
    put16(actual + 2, 0x9A5C);
    put16(actual + 4, count);
    put16(actual + BYTE_AT(block_word) + 6, size);
    memcpy(initial, actual, sizeof(actual));
    expected_owner = actual;
    expected_block = actual + BYTE_AT(block_word);
    expected_size = size;
    call_order = 0;
}

/* The real search dependency runs before any insertion mutation. */
void AllocatorFindNeighbors(void *owner, unsigned short size,
                            void **out_previous, void **out_next)
{
    assert(call_order == 0);
    assert(owner == expected_owner);
    assert(size == expected_size);
    assert(memcmp(actual, initial, sizeof(actual)) == 0);
    *out_previous = expected_previous;
    *out_next = expected_next;
    call_order = 1;
}

/* The real recenter dependency runs after every link and count update. */
void AllocatorRecenter(void *owner)
{
    assert(call_order == 1);
    assert(owner == expected_owner);
    assert(memcmp(actual, expected_at_recenter, sizeof(actual)) == 0);
    put16((unsigned char *)owner + 2, recentered_offset);
    call_order = 2;
}

static void finish_case(void)
{
    put16(expected_final + 2, recentered_offset);
    AllocatorInsertFreeBlock(expected_owner, expected_block);
    assert(call_order == 2);
    assert(memcmp(actual, expected_final, sizeof(actual)) == 0);
}

static void test_empty_list_and_u16_count_wrap(void)
{
    begin_case(0x0100, 0xBEEF, 0, 0xFFFF);
    expected_previous = 0;
    expected_next = 0;
    recentered_offset = 0x2345;

    memcpy(expected_at_recenter, initial, sizeof(actual));
    put16(expected_at_recenter, 0x0100);
    put16(expected_at_recenter + 4, 0x0000);
    put16(expected_at_recenter + BYTE_AT(0x0100) + 8, 0x0000);
    put16(expected_at_recenter + BYTE_AT(0x0100) + 10, 0x0000);
    memcpy(expected_final, expected_at_recenter, sizeof(actual));
    finish_case();
}

static void test_head_insert_with_nonnull_successor(void)
{
    begin_case(0x0120, 0x80F1, 0x0100, 0x12FF);
    expected_previous = 0;
    expected_next = actual + BYTE_AT(0x0100);
    put16(actual + BYTE_AT(0x0100) + 8, 0x7777);
    memcpy(initial, actual, sizeof(actual));
    recentered_offset = 0x0100;

    memcpy(expected_at_recenter, initial, sizeof(actual));
    put16(expected_at_recenter, 0x0120);
    put16(expected_at_recenter + 4, 0x1300);
    put16(expected_at_recenter + BYTE_AT(0x0100) + 8, 0x0120);
    put16(expected_at_recenter + BYTE_AT(0x0120) + 8, 0x0000);
    put16(expected_at_recenter + BYTE_AT(0x0120) + 10, 0x0100);
    memcpy(expected_final, expected_at_recenter, sizeof(actual));
    finish_case();
}

static void test_middle_insert_updates_both_neighbors(void)
{
    begin_case(0x0150, 0xCAFE, 0x0100, 0xABCD);
    expected_previous = actual + BYTE_AT(0x0120);
    expected_next = actual + BYTE_AT(0x0180);
    put16(actual + BYTE_AT(0x0120) + 10, 0x3333);
    put16(actual + BYTE_AT(0x0180) + 8, 0x4444);
    memcpy(initial, actual, sizeof(actual));
    recentered_offset = 0x0180;

    memcpy(expected_at_recenter, initial, sizeof(actual));
    put16(expected_at_recenter + 4, 0xABCE);
    put16(expected_at_recenter + BYTE_AT(0x0120) + 10, 0x0150);
    put16(expected_at_recenter + BYTE_AT(0x0180) + 8, 0x0150);
    put16(expected_at_recenter + BYTE_AT(0x0150) + 8, 0x0120);
    put16(expected_at_recenter + BYTE_AT(0x0150) + 10, 0x0180);
    memcpy(expected_final, expected_at_recenter, sizeof(actual));
    finish_case();
}

static void test_tail_insert_with_null_successor(void)
{
    begin_case(0x0180, 0x0102, 0x0100, 0x00FE);
    expected_previous = actual + BYTE_AT(0x0100);
    expected_next = 0;
    put16(actual + BYTE_AT(0x0100) + 10, 0x5555);
    memcpy(initial, actual, sizeof(actual));
    recentered_offset = 0x0180;

    memcpy(expected_at_recenter, initial, sizeof(actual));
    put16(expected_at_recenter + 4, 0x00FF);
    put16(expected_at_recenter + BYTE_AT(0x0100) + 10, 0x0180);
    put16(expected_at_recenter + BYTE_AT(0x0180) + 8, 0x0100);
    put16(expected_at_recenter + BYTE_AT(0x0180) + 10, 0x0000);
    memcpy(expected_final, expected_at_recenter, sizeof(actual));
    finish_case();
}

int main(void)
{
    assert(sizeof(AllocatorInsertBlockHeader) == 8);
    assert(offsetof(AllocatorInsertBlockHeader, size) == 6);
    test_empty_list_and_u16_count_wrap();
    test_head_insert_with_nonnull_successor();
    test_middle_insert_updates_both_neighbors();
    test_tail_insert_with_null_successor();
    puts("allocator insert whole-buffer checks passed");
    return 0;
}
