#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "unit_reference.h"

static const u8 a[] = {0x11}, b[] = {0x22}, c[] = {0x33}, d[] = {0x44};
/* Large sparse host fixtures exercise encoded boundaries; these sizes are
 * NOT assertions about the original game's table extents. */
const u8 *const gUnitReferenceTable0[65537] = {
    [0] = a, [1] = b, [32767] = c, [65536] = d
};
const u8 *const gUnitReferenceTable1[65537] = {
    [0] = d, [1] = c, [32767] = b, [65536] = a
};
u8 gUnitReferenceFallback[4] = {0x51, 0x62, 0x73, 0x84};

struct GuardedUnit { u32 before; struct UnitRecord unit; u32 after; };
struct UnalignedFixture {
    u32 before;
    u8 leading;
    struct EncodedUnitReference encoded;
    u8 after[7];
};

int main(void)
{
    struct GuardedUnit actual, expected;
    struct UnalignedFixture encoded, expectedEncoded;
    static const struct { u32 value; const u8 *expected; } cases[] = {
        {0, a}, {1, b}, {2, 0}, {0x7fff, c},
        {0x8000, d}, {0x8001, c}, {0x8002, 0}, {0xffff, b},
        {0x10000, d}, {0x10001, c}, {0x80000000u, d},
        {0xffffffffu, b}
    };
    u8 fallbackBefore[4];
    unsigned mode, i;
    assert(sizeof(struct UnitRecord) == 0x108);
    assert(offsetof(struct UnitRecord, unknownFC) + 10 == 0x106);
    assert(offsetof(struct UnalignedFixture, encoded) == 5);
    assert(sizeof(struct EncodedUnitReference) == 4);
    memcpy(fallbackBefore, gUnitReferenceFallback, sizeof(fallbackBefore));

    for (mode = 0; mode < 256; mode++) {
        const u8 *want;
        memset(&actual, 0xA5, sizeof(actual));
        actual.before = 0x13579BDF;
        actual.after = 0x2468ACE0;
        actual.unit.unknownFC[10] = mode;
        /* Invalid index is deliberate for fallback modes: no table access. */
        actual.unit.fields.word00 = mode < 2 ? 1 : 0xffffffffu;
        expected = actual;
        want = mode == 0 ? b : mode == 1 ? c : gUnitReferenceFallback;
        assert(ResolveUnitReference(&actual.unit) == want);
        assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
    }
    for (mode = 0; mode < 2; mode++) {
        actual.unit.unknownFC[10] = mode;
        actual.unit.fields.word00 = 2;
        expected = actual;
        assert(ResolveUnitReference(&actual.unit) == 0);
        assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
    }
    /* The ordinary unit path uses the complete word, without packed masking. */
    for (mode = 0; mode < 2; mode++) {
        actual.unit.unknownFC[10] = mode;
        actual.unit.fields.word00 = 65536;
        expected = actual;
        assert(ResolveUnitReference(&actual.unit) == (mode == 0 ? d : a));
        assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
    }
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        u8 *bytes;
        u32 value = cases[i].value;
        memset(&encoded, 0xA5, sizeof(encoded));
        encoded.before = 0x12345678;
        bytes = (u8 *)&encoded.encoded;
        bytes[0] = value;
        bytes[1] = value >> 8;
        bytes[2] = value >> 16;
        bytes[3] = value >> 24;
        expectedEncoded = encoded;
        assert(ResolveEncodedUnitReference(&encoded.encoded) == cases[i].expected);
        assert(memcmp(&encoded, &expectedEncoded, sizeof(encoded)) == 0);
    }
    assert(memcmp(fallbackBefore, gUnitReferenceFallback, sizeof(fallbackBefore)) == 0);
    return 0;
}
