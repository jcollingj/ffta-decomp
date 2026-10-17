#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_coordinate_lookup.h"
#include "list_accessors.h"

#define NODE_COUNT 6

struct Fixture {
    struct ListCountContext context;
    struct Container owners[3];
    struct ListCountNode nodes[NODE_COUNT];
    struct ContentConsumerEntry entries[NODE_COUNT];
    struct UnitFieldView units[NODE_COUNT];
};

typedef char ContextAt10[
    offsetof(struct ListCountContext, unknown10) == 16 ? 1 : -1];
typedef char ContextAt14[
    offsetof(struct ListCountContext, unknown14) == 16 + sizeof(void *) ? 1 : -1];
typedef char ContextAt18[
    offsetof(struct ListCountContext, unknown18) == 16 + 2 * sizeof(void *) ? 1 : -1];
typedef char EntryPositionXAt8[
    sizeof(void *) != 4 || offsetof(struct ContentConsumerEntry, position08) == 8 ? 1 : -1];
typedef char EntryPositionYAtC[
    sizeof(void *) != 4 || offsetof(struct ContentConsumerEntry, position0C) == 12 ? 1 : -1];

static void init_owner(struct Fixture *f, unsigned owner_index,
                       unsigned first, unsigned count)
{
    unsigned j;

    f->owners[owner_index].allocator_owner = (void *)(unsigned long)(0x1100 + owner_index);
    f->owners[owner_index].tree_root = (struct TreeCountNode *)(unsigned long)(0x2200 + owner_index);
    f->owners[owner_index].list_head = count ? &f->nodes[first] : 0;
    for (j = 0; j < count; ++j) {
        unsigned i = first + j;
        f->nodes[i].unknown00 = (ListPayload)(unsigned long)&f->entries[i];
        f->nodes[i].previous = j ? &f->nodes[i - 1] : 0;
        f->nodes[i].next = j + 1 < count ? &f->nodes[i + 1] : 0;
    }
}

static void init_fixture(struct Fixture *f)
{
    unsigned i;

    memset(f, 0xA5, sizeof(*f));
    init_owner(f, 0, 0, 3);
    init_owner(f, 1, 3, 2);
    init_owner(f, 2, 5, 1);
    f->context.unknown10 = &f->owners[0];
    f->context.unknown14 = &f->owners[1];
    f->context.unknown18 = &f->owners[2];
    for (i = 0; i < NODE_COUNT; ++i) {
        f->entries[i].object = &f->units[i];
    }
}

static void set_position(struct Fixture *f, unsigned i, short x, short y)
{
    f->entries[i].position08 = x;
    f->entries[i].position0C = y;
}

static void set_unit_xy(struct Fixture *f, unsigned i, u8 x, u8 y)
{
    f->units[i].bytesF1[5] = x;
    f->units[i].bytesF1[6] = y;
}

static void test_position_lookup_fixed_images(void)
{
    struct Fixture f;
    struct Fixture before;
    struct ContentConsumerEntry *result;

    init_fixture(&f);
    set_position(&f, 0, 32, 64);
    set_position(&f, 1, 64, 64);
    set_position(&f, 2, 32, 64);
    before = f;
    result = FindContentEntryByPosition(&f.context, 1, 2);
    assert(result == &f.entries[0]);
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    /* Signed entry shifts and signed-byte query values distinguish sign. */
    set_position(&f, 0, (short)-32, (short)-64);
    before = f;
    result = FindContentEntryByPosition(&f.context, -1, -2);
    assert(result == &f.entries[0]);
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    /* Query storage is byte-sized: high input bits are truncated. */
    set_position(&f, 0, 32, 64);
    before = f;
    result = FindContentEntryByPosition(&f.context, 257, 258);
    assert(result == &f.entries[0]);
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    set_position(&f, 0, 128 * 32, 0);
    before = f;
    result = FindContentEntryByPosition(&f.context, 128, 0);
    assert(result == 0);
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    set_position(&f, 0, -4096, 0);
    before = f;
    result = FindContentEntryByPosition(&f.context, 128, 0);
    assert(result == &f.entries[0]); /* query byte 128 is signed -128 */
    assert(memcmp(&f, &before, sizeof(f)) == 0);
}

static void test_unit_lookup_and_first_duplicate(void)
{
    struct Fixture f;
    struct Fixture before;
    struct ContentConsumerEntry *result;

    init_fixture(&f);
    set_unit_xy(&f, 0, 0, 7);
    set_unit_xy(&f, 1, 7, 7);
    set_unit_xy(&f, 2, 7, 7);
    before = f;
    result = FindContentEntryByUnitCoordinates(&f.context, -1, 7);
    assert(result == 0); /* signed query -1 does not equal unsigned unit byte */
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    result = FindContentEntryByUnitCoordinates(&f.context, 7, 7);
    assert(result == &f.entries[1]);
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    result = FindContentEntryByUnitCoordinates(&f.context, 263, 7);
    assert(result == &f.entries[1]); /* input byte truncation */
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    set_unit_xy(&f, 0, 255, 128);
    set_unit_xy(&f, 1, 127, 127);
    set_unit_xy(&f, 2, 128, 255);
    before = f;
    result = FindContentEntryByUnitCoordinates(&f.context, 255, -128);
    assert(result == 0); /* signed -1/-128 do not equal unsigned 255/128 */
    assert(memcmp(&f, &before, sizeof(f)) == 0);
    result = FindContentEntryByUnitCoordinates(&f.context, 127, 127);
    assert(result == &f.entries[1]);
    assert(memcmp(&f, &before, sizeof(f)) == 0);
}

static void test_context_offset_empty_and_absent(void)
{
    struct Fixture f;
    struct Fixture before;
    struct ContentConsumerEntry *result;

    init_fixture(&f);
    set_position(&f, 3, 32, 64); /* +14 decoy only */
    set_position(&f, 5, 32, 64); /* +18 decoy only */
    set_unit_xy(&f, 3, 9, 10); /* +14 decoy only */
    set_unit_xy(&f, 5, 9, 10); /* +18 decoy only */
    before = f;
    result = FindContentEntryByPosition(&f.context, 1, 2);
    assert(result == 0);
    assert(memcmp(&f, &before, sizeof(f)) == 0);
    result = FindContentEntryByUnitCoordinates(&f.context, 9, 10);
    assert(result == 0);
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    f.context.unknown10 = &f.owners[1];
    before = f;
    result = FindContentEntryByPosition(&f.context, 1, 2);
    assert(result == &f.entries[3]);
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    set_unit_xy(&f, 3, 9, 10);
    f.context.unknown10 = &f.owners[1];
    before = f;
    result = FindContentEntryByUnitCoordinates(&f.context, 9, 10);
    assert(result == &f.entries[3]);
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    f.owners[0].list_head = 0;
    f.context.unknown10 = &f.owners[0];
    before = f;
    result = FindContentEntryByUnitCoordinates(&f.context, 1, 2);
    assert(result == 0);
    assert(memcmp(&f, &before, sizeof(f)) == 0);

    before = f;
    result = FindContentEntryByPosition(&f.context, 1, 2);
    assert(result == 0);
    assert(memcmp(&f, &before, sizeof(f)) == 0);
}

int main(void)
{
    assert(sizeof(void *) == 4 || sizeof(void *) == 8);
    test_position_lookup_fixed_images();
    test_unit_lookup_and_first_duplicate();
    test_context_offset_empty_and_absent();
    return 0;
}
