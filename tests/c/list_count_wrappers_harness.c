#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/list_count_wrappers.h"
#include "../../src/list_count.c"
#include "../../src/list_count_wrappers.c"

struct GuardedContext {
    u32 before[4];
    struct ListCountContext value;
    u32 after[4];
};

int main(void)
{
    struct GuardedContext guarded;
    struct GuardedContext guarded_before;
    struct ListCountOwner owners[3];
    struct ListCountOwner owners_before[3];
    struct ListCountNode nodes[303];
    struct ListCountNode nodes_before[303];
    unsigned i;

    /* Host fixture uses natural host pointer width and alignment. */
    assert(offsetof(struct ListCountContext, unknown00) == 0);
    assert(offsetof(struct ListCountContext, unknown10) == 16);
    assert(offsetof(struct ListCountContext, unknown14) == 16 + sizeof(void *));
    assert(offsetof(struct ListCountContext, unknown18) == 16 + 2 * sizeof(void *));

    memset(&guarded, 0xA5, sizeof(guarded));
    memset(owners, 0x3C, sizeof(owners));
    memset(nodes, 0x69, sizeof(nodes));
    owners[0].unknown00 = 0x10203040;
    owners[0].head = 0;
    owners[1].unknown00 = 0x50607080;
    owners[1].head = &nodes[0];
    owners[2].unknown00 = 0x90ABCDEF;
    owners[2].head = &nodes[3];
    for (i = 0; i < 3; i++)
        nodes[i].next = i == 2 ? 0 : &nodes[i + 1];
    for (i = 3; i < 303; i++)
        nodes[i].next = i == 302 ? 0 : &nodes[i + 1];

    guarded.value.unknown10 = &owners[0];
    guarded.value.unknown14 = &owners[1];
    guarded.value.unknown18 = &owners[2];
    guarded_before = guarded;
    memcpy(owners_before, owners, sizeof(owners));
    memcpy(nodes_before, nodes, sizeof(nodes));

    assert(CountContextListAt10(&guarded.value) == 0);
    assert(CountContextListAt14(&guarded.value) == 3);
    assert(CountContextListAt18(&guarded.value) == 300);
    assert(memcmp(&guarded_before, &guarded, sizeof(guarded)) == 0);
    assert(memcmp(owners_before, owners, sizeof(owners)) == 0);
    assert(memcmp(nodes_before, nodes, sizeof(nodes)) == 0);

    /* Swapping distinct fields makes an incorrectly reused offset visible. */
    guarded.value.unknown10 = &owners[2];
    guarded.value.unknown18 = &owners[0];
    guarded_before = guarded;
    assert(CountContextListAt10(&guarded.value) == 300);
    assert(CountContextListAt14(&guarded.value) == 3);
    assert(CountContextListAt18(&guarded.value) == 0);
    assert(memcmp(&guarded_before, &guarded, sizeof(guarded)) == 0);
    assert(memcmp(owners_before, owners, sizeof(owners)) == 0);
    assert(memcmp(nodes_before, nodes, sizeof(nodes)) == 0);
    return 0;
}
