#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/list_count.h"
#include "../../src/list_count.c"

int main(void)
{
    struct ListCountOwner owner;
    struct ListCountNode nodes[1000];
    struct ListCountNode decoys[3];
    struct ListCountOwner owner_before;
    struct ListCountNode nodes_before[1000];
    struct ListCountNode decoys_before[3];
    static const unsigned lengths[] = {0, 1, 2, 5, 255, 256, 1000};
    unsigned test;

    /* Host fixtures use host pointer alignment and native pointer width. */
    assert(sizeof(void *) != 8 || offsetof(struct ListCountOwner, head) == 8);
    assert(offsetof(struct ListCountNode, next) == 8);

    for (test = 0; test < sizeof(lengths) / sizeof(lengths[0]); test++) {
        unsigned i;
        unsigned length = lengths[test];
        memset(&owner, 0xA5, sizeof(owner));
        memset(nodes, 0x5A, sizeof(nodes));
        memset(decoys, 0xC3, sizeof(decoys));
        owner.unknown00 = 0xD00DFEED;
        owner.head = length ? &nodes[0] : 0;
        for (i = 0; i < length; i++)
            nodes[i].next = i + 1 < length ? &nodes[i + 1] : 0;
        /* These nodes are valid but unlinked; they must not affect the count. */
        decoys[0].next = &decoys[1];
        decoys[1].next = &decoys[2];
        decoys[2].next = 0;
        owner_before = owner;
        memcpy(nodes_before, nodes, sizeof(nodes));
        memcpy(decoys_before, decoys, sizeof(decoys));

        assert(CountLinkedNodes(&owner) == length);
        assert(memcmp(&owner_before, &owner, sizeof(owner)) == 0);
        assert(memcmp(nodes_before, nodes, sizeof(nodes)) == 0);
        assert(memcmp(decoys_before, decoys, sizeof(decoys)) == 0);
    }
    return 0;
}
