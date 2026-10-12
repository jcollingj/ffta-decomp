#include "allocator_search.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u8 sBuffer[1200];
static u8 sBefore[sizeof(sBuffer)];
static int sSentinelA;
static int sSentinelB;

static void fail(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

static void put16(u8 *bytes, unsigned offset, u16 value)
{
    bytes[offset] = (u8)value;
    bytes[offset + 1] = (u8)(value >> 8);
}

static u8 *node(unsigned wordOffset)
{
    return sBuffer + wordOffset * 4;
}

static void initialize(const u16 *sizes, unsigned count, unsigned headIndex)
{
    static const unsigned wordOffsets[] = { 8, 20, 32, 44, 0x100 };
    unsigned i;

    if (count > sizeof(wordOffsets) / sizeof(wordOffsets[0]) ||
        (count != 0 && headIndex >= count))
        fail("fixture node count or head index is out of bounds");
    memset(sBuffer, 0xA7, sizeof(sBuffer));
    if (count == 0) {
        put16(sBuffer, 2, 0);
        return;
    }
    put16(sBuffer, 2, (u16)wordOffsets[headIndex]);
    for (i = 0; i < count; ++i) {
        u8 *entry = node(wordOffsets[i]);
        put16(entry, 6, sizes[i]);
        put16(entry, 8, i == 0 ? 0 : (u16)wordOffsets[i - 1]);
        put16(entry, 10, i + 1 == count ? 0 : (u16)wordOffsets[i + 1]);
    }
    memcpy(sBefore, sBuffer, sizeof(sBuffer));
}

static void check_case(const char *name, const u16 *sizes, unsigned count,
                       unsigned headIndex, u16 requested,
                       int expectedA, int expectedB)
{
    void *outA = &sSentinelA;
    void *outB = &sSentinelB;
    static const unsigned wordOffsets[] = { 8, 20, 32, 44, 0x100 };
    void *wantA = expectedA < 0 ? 0 : node(wordOffsets[expectedA]);
    void *wantB = expectedB < 0 ? 0 : node(wordOffsets[expectedB]);

    initialize(sizes, count, headIndex);
    memcpy(sBefore, sBuffer, sizeof(sBuffer));
    AllocatorFindNeighbors(sBuffer, requested, &outA, &outB);
    if (outA != wantA || outB != wantB) {
        fprintf(stderr, "%s: got (%p, %p), expected (%p, %p)\n",
                name, outA, outB, wantA, wantB);
        exit(1);
    }
    if (memcmp(sBuffer, sBefore, sizeof(sBuffer)) != 0)
        fail("allocator search modified bytes in its owner buffer");
}

static void test_alias_store_order(void)
{
    static const u16 sizes[] = { 2, 5 };
    void *out = &sSentinelA;
    void *expected = node(20);

    initialize(sizes, 2, 0);
    memcpy(sBefore, sBuffer, sizeof(sBuffer));
    AllocatorFindNeighbors(sBuffer, 3, &out, &out);
    if (out != expected)
        fail("aliased output stores did not leave the second output value");
    if (memcmp(sBuffer, sBefore, sizeof(sBuffer)) != 0)
        fail("aliased output case modified owner bytes");
}

int main(void)
{
    static const u16 ascending[] = { 2, 5, 9 };
    static const u16 withZero[] = { 0, 5 };
    static const u16 one[] = { 7 };
    static const u16 irregular[] = { 10, 1, 8, 4 };
    static const u16 highBytes[] = { 2, 5, 9, 100, 0x1234 };

    check_case("empty owner", 0, 0, 0, 4, -1, -1);
    check_case("forward zero request", withZero, 2, 0, 0, -1, 0);
    check_case("one node below", one, 1, 0, 6, -1, 0);
    check_case("one node equal", one, 1, 0, 7, -1, 0);
    check_case("one node above", one, 1, 0, 8, 0, -1);
    check_case("first threshold equality", ascending, 3, 0, 2, -1, 0);
    check_case("first forward gap", ascending, 3, 0, 3, 0, 1);
    check_case("middle threshold equality", ascending, 3, 0, 5, 0, 1);
    check_case("last forward gap", ascending, 3, 0, 6, 1, 2);
    check_case("last threshold equality", ascending, 3, 0, 9, 1, 2);
    check_case("above last", ascending, 3, 0, 10, 2, -1);
    check_case("backward multi-step", ascending, 3, 2, 4, 0, 1);
    check_case("backward adjacent equality", ascending, 3, 2, 5, 0, 1);
    check_case("backward head equality", ascending, 3, 2, 9, 1, 2);
    check_case("backward head above", ascending, 3, 2, 10, 2, -1);
    check_case("starting-node branch does not scan later fit", irregular, 4,
               0, 5, -1, 0);
    check_case("high-byte size and next offset", highBytes, 5, 0,
               0x1234, 3, 4);
    check_case("high-byte starting offset", highBytes, 5, 4,
               0xFFFF, 4, -1);
    test_alias_store_order();
    puts("allocator search behavior checks passed");
    return 0;
}
