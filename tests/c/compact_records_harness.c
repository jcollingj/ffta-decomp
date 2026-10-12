#include <assert.h>
#include <string.h>
#include "../../src/nonmatching/compact_records.c"

u8 gSaveSection21A0[1024 + 0x968];
static unsigned copies, clears;
static u8 *copy_dest[64];
static const u8 *copy_src[64];
static u32 copy_size[64];
static u8 *clear_dest[64];
static u32 clear_size[64];
static unsigned events[128];
static unsigned event_count;
static u8 *model_copy_dest[64];
static const u8 *model_copy_src[64];
static u32 model_copy_size[64];
static u8 *model_clear_dest[64];

static void copy_memory(void *dest, const void *src, u32 size)
{
    assert(copies < 64);
    copy_dest[copies] = dest;
    copy_src[copies] = src;
    copy_size[copies++] = size;
    events[event_count++] = 1;
    memmove(dest, src, size);
}

static void clear_memory(void *dest, u32 size)
{
    assert(clears < 64);
    clear_dest[clears] = dest;
    clear_size[clears++] = size;
    events[event_count++] = 2;
    memset(dest, 0, size);
}

void (*const gCopyRecordMemory)(void *, const void *, u32) = copy_memory;
void (*const gClearMemory)(void *, u32) = clear_memory;

static u8 *records(void) { return gSaveSection21A0 + 0x968; }

static void run_case(const unsigned *ids, unsigned count)
{
    u8 before[256], expected[256];
    unsigned occupied = 0, expected_calls = 0, i, holes = 0, output = 0;
    memset(gSaveSection21A0, 0xA5, sizeof(gSaveSection21A0));
    memset(gSaveSection21A0 + 968, 0x5c, 256); /* Decoy: decimal 968 is not the ROM offset. */
    for (i = 0; i < 64; i++) {
        records()[i * 4] = 0;
        records()[i * 4 + 1] = (u8)(0x80 + i);
        records()[i * 4 + 2] = (u8)(0x40 + i);
        records()[i * 4 + 3] = (u8)(0x20 + i);
    }
    for (i = 0; i < count; i++) {
        unsigned id = ids[i];
        if (id != 0) {
            records()[i * 4] = (u8)id;
            records()[i * 4 + 1] = (u8)(id ^ 0x5a);
            records()[i * 4 + 2] = (u8)(id + 37);
            records()[i * 4 + 3] = (u8)(id ^ 0xc3);
            occupied++;
        }
    }
    memcpy(before, records(), sizeof(before));
    memcpy(expected, before, sizeof(expected));
    i = 0;
    while (i < occupied) {
        if (expected[i * 4] == 0) {
            u32 length;
            holes++;
            length = (64 - i - holes) * 4;
            model_copy_dest[expected_calls] = expected + i * 4;
            model_copy_src[expected_calls] = expected + (i + 1) * 4;
            model_copy_size[expected_calls] = length;
            model_clear_dest[expected_calls] = expected + (64 - holes) * 4;
            memmove(expected + i * 4, expected + (i + 1) * 4, length);
            memset(expected + (64 - holes) * 4, 0, 4);
            expected_calls++;
        } else {
            i++;
        }
    }
    copies = clears = event_count = 0;
    assert(CompactSaveRecords64() == (int)occupied);
    assert(records() == gSaveSection21A0 + 0x968);
    assert(memcmp(gSaveSection21A0 + 968, (u8[256]){[0 ... 255] = 0x5c}, 256) == 0);
    assert(memcmp(records(), expected, sizeof(expected)) == 0);
    assert(memcmp(gSaveSection21A0 + 1224,
                  (u8[0x968 - 1224]){[0 ... 0x968 - 1225] = 0xA5},
                  0x968 - 1224) == 0);
    for (i = 0; i < 64; i++) {
        if (before[i * 4] != 0) {
            assert(output < occupied);
            assert(memcmp(records() + output * 4, before + i * 4, 4) == 0);
            output++;
        }
    }
    assert(output == occupied);
    assert(memcmp(gSaveSection21A0, (u8[968]){[0 ... 967] = 0xA5}, 968) == 0);
    assert(memcmp(records() + 256, (u8[768]){[0 ... 767] = 0xA5}, 768) == 0);
    if (occupied == 1 && ids[0] == 1)
        assert(records()[4] == 0 && records()[5] == 0x81);
    assert(copies == expected_calls && clears == expected_calls);
    assert(event_count == expected_calls * 2);
    for (i = 0; i < expected_calls; i++) {
        assert(events[i * 2] == 1 && events[i * 2 + 1] == 2);
        assert(copy_dest[i] == records() + (model_copy_dest[i] - expected));
        assert(copy_src[i] == records() + (model_copy_src[i] - expected));
        assert(copy_size[i] == model_copy_size[i]);
        assert(clear_dest[i] == records() + (model_clear_dest[i] - expected));
        assert(clear_size[i] == 4);
    }
}

int main(void)
{
    unsigned ids[64], n, i;
    run_case(ids, 0); /* Empty array: no callbacks. */
    for (n = 1; n <= 64; n++) {
        for (i = 0; i < 64; i++) ids[i] = 0;
        /* Distinct occupied IDs spread across the complete array. */
        for (i = 0; i < n; i++) ids[(i * 37) % 64] = i + 1;
        run_case(ids, 64);
    }
    for (i = 0; i < 64; i++) ids[i] = i + 1;
    run_case(ids, 64); /* Full array: no callbacks. */
    return 0;
}
