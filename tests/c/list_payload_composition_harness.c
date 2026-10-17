#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "allocator_wrappers.h"
#include "list_accessors.h"
#include "list_append.h"
#include "list_find_word.h"

#define NODE_COUNT 8

struct PayloadObject {
    u32 marker;
    u32 guard;
};

static struct Container owner;
static struct Container expected_owner;
static struct ListCountNode nodes[NODE_COUNT];
static struct ListCountNode expected_nodes[NODE_COUNT];
static struct PayloadObject objects[2] = {
    {0x13579BDFu, 0x2468ACE0u},
    {0xA1B2C3D4u, 0x5E6F7081u}
};
static const struct PayloadObject expected_objects[2] = {
    {0x13579BDFu, 0x2468ACE0u},
    {0xA1B2C3D4u, 0x5E6F7081u}
};
static int allocator_tag;
static unsigned allocation_index;

typedef char PayloadAtOffsetZero[
    offsetof(struct ListCountNode, unknown00) == 0 ? 1 : -1];
typedef char PreviousAtNativeWord[
    offsetof(struct ListCountNode, previous) == sizeof(void *) ? 1 : -1];
typedef char NextAtTwoNativeWords[
    offsetof(struct ListCountNode, next) == 2 * sizeof(void *) ? 1 : -1];
typedef char NativeListNodeSize[
    sizeof(struct ListCountNode) == 3 * sizeof(void *) ? 1 : -1];

static ListPayload PayloadFromPointer(const void *pointer)
{
    return (ListPayload)(unsigned long)pointer;
}

static void *PointerFromPayload(ListPayload payload)
{
    return (void *)(unsigned long)payload;
}

void *AllocatorAllocate(void *allocator_owner, u32 size)
{
    assert(allocator_owner == &allocator_tag);
    assert(size == 12);
    assert(allocation_index < NODE_COUNT);
    return &nodes[allocation_index++];
}

static void AssertWholeImage(unsigned expected_allocations)
{
    assert(allocation_index == expected_allocations);
    assert(memcmp(&owner, &expected_owner, sizeof(owner)) == 0);
    assert(memcmp(nodes, expected_nodes, sizeof(nodes)) == 0);
}

int main(void)
{
    ListPayload pointer_a = PayloadFromPointer(&objects[0]);
    ListPayload pointer_b = PayloadFromPointer(&objects[1]);
    struct ListCountNode *result;
    void *decoded;

    assert(sizeof(ListPayload) == sizeof(void *));
    assert(pointer_a != pointer_b);
    assert(pointer_a != 0UL && pointer_b != 0UL);
    assert(pointer_a != 0x80000000UL && pointer_b != 0x80000000UL);
    assert(pointer_a != 0xFFFFFFFFUL && pointer_b != 0xFFFFFFFFUL);

    memset(&owner, 0, sizeof(owner));
    memset(&expected_owner, 0, sizeof(expected_owner));
    memset(nodes, 0xA5, sizeof(nodes));
    memset(expected_nodes, 0xA5, sizeof(expected_nodes));
    owner.allocator_owner = &allocator_tag;
    expected_owner.allocator_owner = &allocator_tag;
    allocation_index = 0;

    /* The first real append stores and returns a full native object identity. */
    result = AppendListNode(&owner, pointer_a);
    expected_nodes[0].unknown00 = pointer_a;
    expected_nodes[0].previous = 0;
    expected_nodes[0].next = 0;
    expected_owner.list_head = &nodes[0];
    AssertWholeImage(1);
    assert(result == &nodes[0]);
    assert(GetListOwnerWord0(result) == pointer_a);
    decoded = PointerFromPayload(GetListOwnerWord0(result));
    assert(decoded == &objects[0]);
    assert(((struct PayloadObject *)decoded)->marker == 0x13579BDFu);
    assert(((struct PayloadObject *)decoded)->guard == 0x2468ACE0u);
    assert(FindListNodeByWord(&owner, pointer_a) == &nodes[0]);

    /* A second distinct pointer proves that host pointer bits are not narrowed. */
    result = AppendListNode(&owner, pointer_b);
    expected_nodes[0].next = &nodes[1];
    expected_nodes[1].unknown00 = pointer_b;
    expected_nodes[1].previous = &nodes[0];
    expected_nodes[1].next = 0;
    AssertWholeImage(2);
    assert(result == &nodes[1]);
    decoded = PointerFromPayload(GetListOwnerWord0(result));
    assert(decoded == &objects[1]);
    assert(((struct PayloadObject *)decoded)->marker == 0xA1B2C3D4u);
    assert(((struct PayloadObject *)decoded)->guard == 0x5E6F7081u);
    assert(FindListNodeByWord(&owner, pointer_a) == &nodes[0]);
    assert(FindListNodeByWord(&owner, pointer_b) == &nodes[1]);

    /* Numeric keys retain the original unsigned word boundaries and equality. */
    result = AppendListNode(&owner, 0UL);
    expected_nodes[1].next = &nodes[2];
    expected_nodes[2].unknown00 = 0UL;
    expected_nodes[2].previous = &nodes[1];
    expected_nodes[2].next = 0;
    AssertWholeImage(3);
    assert(result == &nodes[2]);
    assert(GetListOwnerWord0(result) == 0UL);
    assert(FindListNodeByWord(&owner, 0UL) == &nodes[2]);

    result = AppendListNode(&owner, 0x80000000UL);
    expected_nodes[2].next = &nodes[3];
    expected_nodes[3].unknown00 = 0x80000000UL;
    expected_nodes[3].previous = &nodes[2];
    expected_nodes[3].next = 0;
    AssertWholeImage(4);
    assert(result == &nodes[3]);
    assert(GetListOwnerWord0(result) == 0x80000000UL);
    assert(FindListNodeByWord(&owner, 0x80000000UL) == &nodes[3]);

    result = AppendListNode(&owner, 0xFFFFFFFFUL);
    expected_nodes[3].next = &nodes[4];
    expected_nodes[4].unknown00 = 0xFFFFFFFFUL;
    expected_nodes[4].previous = &nodes[3];
    expected_nodes[4].next = 0;
    AssertWholeImage(5);
    assert(result == &nodes[4]);
    assert(GetListOwnerWord0(result) == 0xFFFFFFFFUL);
    assert(FindListNodeByWord(&owner, 0xFFFFFFFFUL) == &nodes[4]);

    result = AppendListNode(&owner, ULONG_MAX);
    expected_nodes[4].next = &nodes[5];
    expected_nodes[5].unknown00 = ULONG_MAX;
    expected_nodes[5].previous = &nodes[4];
    expected_nodes[5].next = 0;
    AssertWholeImage(6);
    assert(result == &nodes[5]);
    assert(GetListOwnerWord0(result) == ULONG_MAX);
#if ULONG_MAX == 0xFFFFFFFFUL
    /* On ILP32, ULONG_MAX intentionally aliases the earlier 32-bit word. */
    assert(FindListNodeByWord(&owner, ULONG_MAX) == &nodes[4]);
#else
    assert(FindListNodeByWord(&owner, ULONG_MAX) == &nodes[5]);
#endif

    /* Duplicate numeric values still return the first matching node. */
    result = AppendListNode(&owner, 0xFFFFFFFFUL);
    expected_nodes[5].next = &nodes[6];
    expected_nodes[6].unknown00 = 0xFFFFFFFFUL;
    expected_nodes[6].previous = &nodes[5];
    expected_nodes[6].next = 0;
    AssertWholeImage(7);
    assert(result == &nodes[6]);
    assert(GetListOwnerWord0(result) == 0xFFFFFFFFUL);
    assert(FindListNodeByWord(&owner, 0xFFFFFFFFUL) == &nodes[4]);
    assert(nodes[7].unknown00 == expected_nodes[7].unknown00);

    /* All final getter/search calls are read-only across the whole fixture. */
    {
        struct Container owner_before_reads = owner;
        struct ListCountNode nodes_before_reads[NODE_COUNT];
        unsigned i;
        memcpy(nodes_before_reads, nodes, sizeof(nodes));
        for (i = 0; i < 7; ++i)
            assert(GetListOwnerWord0(&nodes[i]) == nodes[i].unknown00);
        assert(PointerFromPayload(GetListOwnerWord0(&nodes[0])) == &objects[0]);
        assert(PointerFromPayload(GetListOwnerWord0(&nodes[1])) == &objects[1]);
        assert(FindListNodeByWord(&owner, pointer_a) == &nodes[0]);
        assert(FindListNodeByWord(&owner, pointer_b) == &nodes[1]);
        assert(FindListNodeByWord(&owner, 0UL) == &nodes[2]);
        assert(FindListNodeByWord(&owner, 0x80000000UL) == &nodes[3]);
        assert(FindListNodeByWord(&owner, 0xFFFFFFFFUL) == &nodes[4]);
#if ULONG_MAX == 0xFFFFFFFFUL
        assert(FindListNodeByWord(&owner, ULONG_MAX) == &nodes[4]);
#else
        assert(FindListNodeByWord(&owner, ULONG_MAX) == &nodes[5]);
#endif
        assert(memcmp(&owner, &owner_before_reads, sizeof(owner)) == 0);
        assert(memcmp(nodes, nodes_before_reads, sizeof(nodes)) == 0);
        assert(memcmp(objects, expected_objects, sizeof(objects)) == 0);
    }
    return 0;
}
