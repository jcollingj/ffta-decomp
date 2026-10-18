#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "unit_reference_encode.h"

static const u8 a[] = {1}, b[] = {2}, c[] = {3}, d[] = {4};
static const u8 shared[] = {5}, excluded[] = {6}, missing[] = {7};
/* Extra sentinel entries distinguish an incorrect inclusive upper bound. */
const u8 *const gUnitReferenceTable0[726] = {
    [0] = a, [1] = a, [3] = shared, [6] = shared,
    [724] = b, [725] = excluded
};
const u8 *const gUnitReferenceTable1[108] = {
    [0] = c, [1] = shared, [106] = d, [107] = excluded
};
u8 gUnitReferenceFallback[4] = {9, 8, 7, 6};
struct GuardedPointer {
    u32 before;
    u8 leading;
    struct PackedUnitReferencePointer pointer;
    u8 after[8];
};
struct GuardedIndex {
    u32 before;
    u8 leading;
    struct EncodedUnitReference index;
    u8 after[8];
};

int main(void)
{
    static const struct { const u8 *pointer; u32 encoded; } cases[] = {
        {a, 0}, {b, 724}, {c, 0x8000}, {d, 0x806a},
        {shared, 3}, {0, 2}, {excluded, 0}, {missing, 0}
    };
    unsigned i;
    assert(offsetof(struct GuardedPointer, pointer) == 5);
    assert(offsetof(struct GuardedIndex, index) == 5);
    assert(sizeof(struct PackedUnitReferencePointer) == sizeof(const u8 *));
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        struct GuardedPointer actual, expected;
        struct GuardedIndex index, expectedIndex;
        u32 encoded;
        memset(&actual, 0xA5, sizeof(actual));
        actual.before = 0x11223344;
        actual.pointer.value = cases[i].pointer;
        expected = actual;
        encoded = EncodeUnitReferencePointer(&actual.pointer);
        assert(encoded == cases[i].encoded);
        assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
        if (i < 6) {
            memset(&index, 0x72, sizeof(index));
            index.index.value = encoded;
            expectedIndex = index;
            assert(ResolveEncodedUnitReference(&index.index) == cases[i].pointer);
            assert(memcmp(&index, &expectedIndex, sizeof(index)) == 0);
        }
    }
    return 0;
}
